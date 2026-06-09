/**
 * @file main.c
 * @brief Протокол 5Ei – финальная версия с исправленной записью адреса и сбросом состояний
 */

#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/timer/tmr1.h"
#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 16000000

#define BIT_THRESHOLD_US 450
#define T2_MIN_US 100
#define T2_MAX_US 160

// Простой всей шины дольше ~100 мс => сброс накопителей команд (mode 1/2/3),
// чтобы протухшее частичное состояние не жило вечно. Цикл main = ~100 мкс/итерация,
// поэтому 1000 итераций ~ 100 мс. Порог должен быть выше штатного межфреймового
// зазора 20-35 мс, иначе ложные сбросы.
#define IDLE_RESET_LOOPS 1000

typedef enum {
    IDLE,
    RECEIVING_BITS,
    WAIT_T2,
    CMD_READY
} rx_state_t;

typedef enum {
    AWAIT_FIRST,
    AWAIT_SECOND,
    AWAIT_ADDR1,
    AWAIT_ADDR2
} set_addr_state_t;

volatile rx_state_t rx_state = IDLE;
volatile uint8_t bit_count = 0;
volatile uint32_t rx_buffer = 0;
volatile uint8_t cmd_ready = 0;
volatile uint8_t rx_mode = 0;
volatile uint8_t rx_addr = 0;

volatile set_addr_state_t set_addr_state = AWAIT_FIRST;
volatile uint8_t new_address = 0;
uint8_t my_address = 0x01;

static uint8_t reset_counter = 0;
static uint8_t activate_counter = 0;

volatile uint8_t t2_received = 0;
volatile uint8_t response_phase = 0;
volatile uint16_t prev_ticks = 0;
volatile uint8_t first_measure = 1;

uint8_t eeprom_read(uint8_t addr) {
    EEADRL = addr;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATL;
}

void eeprom_write(uint8_t addr, uint8_t data) {
    EEADRL = addr;
    EEDATL = data;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    INTCONbits.GIE = 1;
    EECON1bits.WREN = 0;
}

void protocol_on_bus_change(void) {
    uint16_t cur = TMR1;
    uint16_t delta;

    // --- Frame sync via inter-frame idle gap ---
    // Panel polls are spaced >=20 ms (spec) > TMR1 overflow period (16.384 ms).
    // TMR1 overflow IRQ is disabled, so a long idle just latches PIR1bits.TMR1IF.
    // Set here => a gap elapsed => THIS edge is the first edge of a new frame.
    if (PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0;
        prev_ticks = cur;
        first_measure = 0;
        bit_count = 0;
        rx_buffer = 0;
        if (rx_state != WAIT_T2) rx_state = RECEIVING_BITS; // don't disturb an in-progress response
        return;                 // seed only: this delta spans the idle gap, not a bit
    }

    if (first_measure) {        // very first edge after boot, no gap measured yet
        first_measure = 0;
        prev_ticks = cur;
        return;
    }

    if (cur >= prev_ticks)
        delta = cur - prev_ticks;
    else
        delta = (65535 - prev_ticks) + cur;
    uint16_t us = delta / 4;
    prev_ticks = cur;

    Y_LED_SetHigh(); __delay_us(30); Y_LED_SetLow();

    if (rx_state == WAIT_T2) {
        if (us >= T2_MIN_US && us <= T2_MAX_US) {
            t2_received = 1;
            Y_LED_SetHigh(); __delay_us(50); Y_LED_SetLow();
        }
        return;
    }

    uint8_t bit = (us < BIT_THRESHOLD_US) ? 0 : 1;

    // Capture 12 data bits: [mode:4 LSB-first][addr:8 LSB-first]; 3 trailing sync bits ignored.
    if (rx_state == RECEIVING_BITS && bit_count < 12) {
        rx_buffer |= (uint32_t)bit << bit_count;
        bit_count++;
        if (bit_count == 12) {
            rx_state = CMD_READY;
            rx_mode = rx_buffer & 0x0F;        // bits 0-3  = MODE  (now the real field)
            rx_addr = (rx_buffer >> 4) & 0xFF; // bits 4-11 = ADDR
            cmd_ready = 1;
            G_LED_SetHigh(); __delay_us(500); G_LED_SetLow();
        }
    }
}

void send_pulse(uint16_t us) {
    DATA_OUT_SetHigh();
    if (us == 800) __delay_us(800);
    else if (us == 1800) __delay_us(1800);
    else __delay_us(1000);
    DATA_OUT_SetLow();
}

void send_type_bits(uint8_t type) {
    for (uint8_t i = 0; i < 4; i++) {
        if (type & (1 << i)) {
            DATA_OUT_SetHigh();
            __delay_us(300);
            DATA_OUT_SetLow();
        } else {
            __delay_us(300);
        }
    }
}

uint16_t read_adc_rb1(void) {
    ADCON1 = 0x80;
    ADCON0 = 0x01;
    ADCON0bits.CHS = 4;
    __delay_us(10);
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    return ((uint16_t)ADRESH << 8) | ADRESL;
}

void send_response_phase(void) {
    uint16_t adc_val = read_adc_rb1();
    uint16_t pulse_us = (adc_val < 20) ? 800 : 1800;
    switch (response_phase) {
        case 0: send_pulse(pulse_us); break;
        case 1: send_type_bits(0x06); break;
        case 2: send_pulse(pulse_us); break;
    }
    G_LED_SetHigh(); __delay_us(100); G_LED_SetLow();
}

void process_command(void) {
    // ========== ОБРАБОТКА MODE 1 (УСТАНОВКА АДРЕСА) ==========
    if (rx_mode == 1 || rx_mode == 8) {
        // Стейт-машина в глобале set_addr_state, чтобы idle-таймаут в main мог её сбросить.
        if (set_addr_state == AWAIT_FIRST && rx_addr == 0x55) set_addr_state = AWAIT_SECOND;
        else if (set_addr_state == AWAIT_SECOND && rx_addr == 0x3A) set_addr_state = AWAIT_ADDR1;
        else if (set_addr_state == AWAIT_ADDR1) { new_address = rx_addr; set_addr_state = AWAIT_ADDR2; }
        else if (set_addr_state == AWAIT_ADDR2 && rx_addr == new_address) {
            eeprom_write(0, new_address);
            my_address = new_address;
            set_addr_state = AWAIT_FIRST;
            // Успех: мигаем зелёным 2 раза
            for (int i = 0; i < 2; i++) {
                G_LED_SetHigh(); __delay_ms(100);
                G_LED_SetLow();  __delay_ms(100);
            }
            // ------------------------------------------------------------
            // СБРОС ВСЕХ СОСТОЯНИЙ, ЧТОБЫ УСТРОЙСТВО ПРОДОЛЖАЛО РАБОТАТЬ
            rx_state = IDLE;
            cmd_ready = 0;
            t2_received = 0;
            response_phase = 0;
            bit_count = 0;
            first_measure = 1;
            prev_ticks = TMR1;
            T1CONbits.TMR1ON = 1;
            PIR1bits.TMR1IF = 0;
            rx_buffer = 0;
            // Небольшая задержка после записи (опционально)
            __delay_ms(10);
            // ------------------------------------------------------------
        } else {
            set_addr_state = AWAIT_FIRST;
        }
        return;
    }
    
    // ========== ОСТАЛЬНЫЕ РЕЖИМЫ ==========
    if (rx_mode == 5 || rx_mode == 6 || rx_mode == 7) {
        for (int i = 0; i < 10; i++) {
            G_LED_SetHigh(); __delay_ms(250);
            G_LED_SetLow();  __delay_ms(250);
        }
        return;
    }
    
    if (rx_addr != my_address) return;
    
    switch (rx_mode) {
        case 0:
        case 4:
            rx_state = WAIT_T2;
            response_phase = 0;
            t2_received = 0;
            G_LED_SetHigh(); __delay_us(200); G_LED_SetLow();
            break;
        case 2:
            reset_counter++;
            if (reset_counter >= 4) { reset_counter = 0; RESET(); }
            break;
        case 3:
            activate_counter++;
            if (activate_counter >= 4) {
                activate_counter = 0;
                LOAD_SetHigh();
                Y_LED_SetHigh();
            }
            break;
        default: break;
    }
}

void main(void) {
    SYSTEM_Initialize();
    G_LED_SetDigitalOutput();
    Y_LED_SetDigitalOutput();
    LOAD_SetDigitalOutput();
    DATA_OUT_SetDigitalOutput();
    DATA_OUT_SetLow();

    for (int i = 0; i < 3; i++) {
        G_LED_SetHigh(); __delay_ms(200);
        G_LED_SetLow();  __delay_ms(200);
    }

    T1CON = 0;
    T1CONbits.TMR1CS = 0;
    T1CONbits.T1CKPS = 0;
    TMR1 = 0;
    PIE1bits.TMR1IE = 0;
    T1CONbits.TMR1ON = 1;

    my_address = eeprom_read(0);
    if (my_address == 0xFF || my_address == 0) my_address = 0x01;

    DATA_IN_SetInterruptHandler(protocol_on_bus_change);
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;

    G_LED_SetHigh(); __delay_ms(100); G_LED_SetLow();

    uint16_t idle_loops = 0;
    while (1) {
        if (cmd_ready) {
            cmd_ready = 0;
            process_command();
            idle_loops = 0;                 // любой фрейм на шине = активность
        } else if (++idle_loops >= IDLE_RESET_LOOPS) {
            // Вся шина молчит дольше порога => сброс протухших накопителей.
            // Не трогает mode 2/3 во время штатного опроса: фреймы к другим
            // адресам тоже сбрасывают idle_loops.
            idle_loops = 0;
            reset_counter = 0;
            activate_counter = 0;
            set_addr_state = AWAIT_FIRST;
        }
        if (rx_state == WAIT_T2 && t2_received) {
            t2_received = 0;
            send_response_phase();
            response_phase++;
            if (response_phase >= 3) {
                rx_state = IDLE;
                response_phase = 0;
                bit_count = 0;
            }
        }
        __delay_us(100);
    }
}