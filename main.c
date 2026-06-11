/**
 * @file main.c
 * @brief Протокол 5Ei — узел пожарного извещателя (slave) на PIC16F.
 *
 * ФИЗИКА ШИНЫ
 *   Однопроводная шина, в покое HIGH. Панель (master) притягивает её в LOW,
 *   чтобы передать кадр. Извещатель отвечает модуляцией тока через DATA_OUT (RA3).
 *   Вход DATA_IN (RB0) — прерывание по изменению (IOC) на ОБА фронта.
 *
 * КОДИРОВАНИЕ DOWNLINK
 *   Линия переключается на каждом бите. Каждый интервал между фронтами = один бит:
 *       интервал < 450 мкс  -> 0  (короткий, ~300 мкс)
 *       интервал >= 450 мкс -> 1  (длинный,  ~670 мкс)
 *   Кадр = 12 бит = mode[4, младшим вперёд] + addr[8, младшим вперёд].
 *   Первый фронт кадра идёт после длинной паузы шины; он используется только как
 *   опорная точка (seed) и битом НЕ считается. Дальше идут 12 бит.
 *
 * СИНХРОНИЗАЦИЯ КАДРОВ  (главное исправление)
 *   Старая версия считала началом кадра только переполнение TMR1 (PIR1bits.TMR1IF).
 *   TMR1 @4 МГц/1:1 переполняется каждые 16.384 мс, а реальная межкадровая пауза
 *   ~16.3 мс — чуть МЕНЬШЕ периода переполнения. Поэтому флаг чаще НЕ выставлялся и
 *   ~88% кадров терялись: режим 0 «глючил» при частом опросе, режим 1 (установка
 *   адреса, нужно 4 кадра подряд) почти никогда не срабатывал.
 *
 *   Теперь граница кадра определяется по ИЗМЕРЕННОМУ интервалу:
 *       gap = (delta >= GAP_TICKS)  ||  переполнение TMR1
 *   Прескалер TMR1 = 1:8 (2 мкс/тик), период переполнения 131 мс — длиннее и кадра
 *   (~13 мс), и паузы (~16 мс). Значит переполнение НЕ попадает на середину кадра
 *   и не вызывает ложный сброс; delta всегда корректно меряет паузу 16 мс (8000
 *   тиков), а TMR1IF взводится только при настоящем молчании шины (> 131 мс).
 *   Порог GAP_TICKS лежит между макс. внутрикадровым интервалом (окно ответа ~2.5 мс)
 *   и минимальной паузой между кадрами (~7..16 мс).
 */

#include "mcc_generated_files/system/system.h"
#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 16000000

/* TMR1 = Fosc/4 (4 МГц) с прескалером 1:8 => 0.5 МГц => 2 мкс на тик.
 * Прескалер 1:8 КРИТИЧЕН: переполнение раз в 131 мс (65536*2 мкс) — заметно
 * длиннее и кадра (~13 мс), и межкадровой паузы (~16 мс). При 1:1 переполнение
 * каждые 16.384 мс попадало ВНУТРЬ кадра и ложно сбрасывало приём бит
 * (ovf=1 на середине кадра) — это и были периодические «затупы» режима 0. */
#define US_TO_TICKS(us)   ((uint16_t)((uint32_t)(us) / 2u))

#define BIT_TICKS    US_TO_TICKS(450)    /* порог 0/1 для бита данных            */
#define GAP_TICKS    US_TO_TICKS(4000)   /* >= этого интервал считается паузой   */

/* Окно метки T2 панели (~170 мкс). ВАЖНО: верхняя граница НИЖЕ 300 мкс, иначе в
 * окно попадают собственные импульсы ответа (type-биты 300 мкс) и короткие
 * импульсы панели -> ложные/преждевременные фазы ("не ждёт синхро"). */
#define T2_MIN_TICKS US_TO_TICKS(100)
#define T2_MAX_TICKS US_TO_TICKS(200)

/* Длинный импульс перед фазами ответа: Sync (~600 мкс) или окно ответа (~2 мс).
 * Пока он не пришёл, метки T2 НЕ обрабатываем — это и есть «ждать синхро». */
#define SYNC_TICKS   US_TO_TICKS(400)

/* Сколько итераций main без единого кадра считать «шина молчит» и сбрасывать
 * накопленные многокадровые состояния (set-address / reset / activation).
 * Холостая итерация ~100 мкс, кадры при опросе идут каждые ~29 мс, полная
 * последовательность установки адреса ~120 мс — порог должен быть заметно выше. */
#define IDLE_RESET_LOOPS  3000           /* ~300 мс */

/* ---- Состояние приёмника (пишется из ISR, читается в main) ---- */
volatile uint8_t  bit_count   = 12;      /* 0..12; 12 = ждём паузу (idle)        */
volatile uint32_t rx_buffer   = 0;
volatile uint16_t prev_ticks  = 0;

volatile uint8_t  cmd_ready   = 0;       /* ISR -> main: пришёл полный кадр       */
volatile uint8_t  rx_mode     = 0;
volatile uint8_t  rx_addr     = 0;

volatile uint8_t  in_response = 0;       /* идёт последовательность ответа        */
volatile uint8_t  resp_armed  = 0;       /* увидели Sync/окно — теперь ловим метки */
volatile uint8_t  resp_phase  = 0;       /* 0=Smoke, 1=Type, 2=Heat               */
volatile uint8_t  resp_request= 0;       /* ISR -> main: пришла метка, шлём фазу   */

/* ---- Многокадровые состояния (обрабатываются в main) ---- */
typedef enum { AWAIT_FIRST, AWAIT_SECOND, AWAIT_ADDR1, AWAIT_ADDR2 } set_addr_state_t;
set_addr_state_t set_addr_state = AWAIT_FIRST;
uint8_t new_address    = 0;
uint8_t my_address     = 0x01;
uint8_t reset_counter  = 0;
uint8_t activate_counter = 0;

/* =========================================================================
 *  EEPROM
 * ========================================================================= */
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
    INTCONbits.GIE = 0;          /* разблокировка записи требует атомарности */
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    INTCONbits.GIE = 1;
    EECON1bits.WREN = 0;
}

/* =========================================================================
 *  Приёмник кадров — обработчик прерывания по фронту на DATA_IN (RB0)
 *  Никаких блокирующих задержек: они сбивают измерение следующего интервала.
 * ========================================================================= */
void protocol_on_bus_change(void) {
    /* Чтение 16-битного TMR1 без «разрыва»: если младший байт перескочил во время
     * чтения, старший мог измениться — перечитываем. Иначе возможна ошибка на
     * 256 тиков (512 мкс при 1:8), которой хватает, чтобы перепутать 0/1 бита. */
    uint8_t hi = TMR1H;
    uint8_t lo = TMR1L;
    if (TMR1H != hi) { hi = TMR1H; lo = TMR1L; }
    uint16_t cur = ((uint16_t)hi << 8) | lo;

    uint8_t  ovf = PIR1bits.TMR1IF;
    PIR1bits.TMR1IF = 0;

    uint16_t delta = (uint16_t)(cur - prev_ticks);   /* 16-битная арифметика с заворотом */
    prev_ticks = cur;

    /* --- Граница кадра: длинная пауза шины --- */
    if (ovf || delta >= GAP_TICKS) {
        bit_count    = 0;        /* этот фронт — начало кадра (seed), не бит */
        rx_buffer    = 0;
        in_response  = 0;        /* новый кадр отменяет незавершённый ответ  */
        resp_armed   = 0;
        resp_phase   = 0;
        return;
    }

    /* --- Фаза ответа --- */
    if (in_response) {
        if (!resp_armed) {
            /* Сначала дожидаемся Sync/окна (длинный импульс) — НЕ реагируем на
             * короткие импульсы, пока панель не открыла слот ответа. */
            if (delta >= SYNC_TICKS) resp_armed = 1;
        } else if (delta >= T2_MIN_TICKS && delta <= T2_MAX_TICKS) {
            resp_request = 1;    /* метка T2 -> main выдаёт очередную фазу */
        }
        return;
    }

    /* --- Приём 12 бит команды --- */
    if (bit_count < 12) {
        uint8_t bit = (delta >= BIT_TICKS) ? 1u : 0u;
        rx_buffer |= (uint32_t)bit << bit_count;
        bit_count++;
        if (bit_count == 12) {
            rx_mode  = (uint8_t)(rx_buffer & 0x0F);          /* биты 0..3  = MODE */
            rx_addr  = (uint8_t)((rx_buffer >> 4) & 0xFF);   /* биты 4..11 = ADDR */
            cmd_ready = 1;       /* остальные фронты кадра игнорируются (bit_count==12) */
        }
    }
}

/* =========================================================================
 *  Передатчик ответа (Mode 0/4) — модуляция тока на DATA_OUT
 * ========================================================================= */
void send_pulse(uint16_t us) {
    DATA_OUT_SetHigh();
    if      (us == 800)  __delay_us(800);
    else if (us == 1800) __delay_us(1800);
    else                 __delay_us(1000);
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

void send_response_phase(uint8_t phase) {
    uint16_t adc_val  = read_adc_rb1();
    uint16_t pulse_us = (adc_val < 20) ? 800 : 1800;
    switch (phase) {
        case 0: send_pulse(pulse_us); break;   /* Smoke */
        case 1: send_type_bits(0x06); break;   /* Type = выходной модуль */
        case 2: send_pulse(pulse_us); break;   /* Heat  */
        default: break;
    }
}

/* =========================================================================
 *  Сброс многокадровых накопителей (по «молчанию» шины)
 * ========================================================================= */
static void reset_sequencers(void) {
    set_addr_state   = AWAIT_FIRST;
    reset_counter    = 0;
    activate_counter = 0;
}

/* =========================================================================
 *  Обработка принятой команды (вызывается из main)
 * ========================================================================= */
void process_command(void) {

    /* ===== MODE 1: установка адреса (55h -> 3Ah -> addr -> addr) ===== */
    if (rx_mode == 1) {
        switch (set_addr_state) {
            case AWAIT_FIRST:
                set_addr_state = (rx_addr == 0x55) ? AWAIT_SECOND : AWAIT_FIRST;
                break;
            case AWAIT_SECOND:
                set_addr_state = (rx_addr == 0x3A) ? AWAIT_ADDR1 : AWAIT_FIRST;
                break;
            case AWAIT_ADDR1:
                new_address    = rx_addr;
                set_addr_state = AWAIT_ADDR2;
                break;
            case AWAIT_ADDR2:
                if (rx_addr == new_address) {
                    eeprom_write(0, new_address);
                    my_address = new_address;
                    /* подтверждение: два зелёных мигания */
                    for (uint8_t i = 0; i < 2; i++) {
                        G_LED_SetHigh(); __delay_ms(100);
                        G_LED_SetLow();  __delay_ms(100);
                    }
                }
                set_addr_state = AWAIT_FIRST;
                break;
        }
        return;
    }

    /* Любая НЕ-mode-1 команда прерывает набор адреса. */
    set_addr_state = AWAIT_FIRST;

    /* ===== Команды, адресованные конкретному узлу ===== */
    if (rx_addr != my_address) {
        /* Кадр не нам — сбрасываем счётчики кратных команд, чтобы чужой опрос
         * не «докручивал» наши reset/activation. */
        reset_counter    = 0;
        activate_counter = 0;
        return;
    }

    switch (rx_mode) {
        case 0:                      /* чтение/опрос */
        case 4:                      /* ответ извещателя */
            in_response = 1;         /* ждём Sync/окно, затем фазы по меткам T2 */
            resp_armed  = 0;
            resp_phase  = 0;
            resp_request= 0;
            reset_counter    = 0;
            activate_counter = 0;
            break;

        case 2:                      /* Reset — 4 команды подряд */
            activate_counter = 0;
            if (++reset_counter >= 4) { reset_counter = 0; RESET(); }
            break;

        case 3:                      /* Activation — 4 команды подряд */
            reset_counter = 0;
            if (++activate_counter >= 4) {
                activate_counter = 0;
                LOAD_SetHigh();
                Y_LED_SetHigh();
            }
            break;

        case 5:                      /* Calibrate Heat / Test / Calibrate Smoke */
        case 6:
        case 7:
            reset_counter    = 0;
            activate_counter = 0;
            /* короткая индикация без длинной блокировки шины */
            G_LED_SetHigh(); __delay_ms(60); G_LED_SetLow();
            break;

        default:
            reset_counter    = 0;
            activate_counter = 0;
            break;
    }
}

/* =========================================================================
 *  main
 * ========================================================================= */
void main(void) {
    SYSTEM_Initialize();

    G_LED_SetDigitalOutput();
    Y_LED_SetDigitalOutput();
    R_LED_SetDigitalOutput();
    LOAD_SetDigitalOutput();
    DATA_OUT_SetDigitalOutput();
    DATA_OUT_SetLow();

    /* стартовая индикация */
    for (uint8_t i = 0; i < 3; i++) {
        G_LED_SetHigh(); __delay_ms(150);
        G_LED_SetLow();  __delay_ms(150);
    }

    /* TMR1: Fosc/4 = 4 МГц, прескалер 1:8 => 2 мкс/тик, переполнение раз в 131 мс.
     * Период переполнения СПЕЦИАЛЬНО длиннее кадра и паузы, чтобы переполнение
     * не приходилось на середину кадра (иначе ложный сброс приёма). TMR1IF теперь
     * взводится только при «настоящем» молчании шины (> 131 мс). */
    T1CON = 0;
    T1CONbits.TMR1CS  = 0;       /* источник Fosc/4 */
    T1CONbits.T1CKPS  = 3;       /* 1:8             */
    T1CONbits.nT1SYNC = 1;
    TMR1 = 0;
    PIE1bits.TMR1IE = 0;         /* переполнение НЕ генерирует прерывание */
    T1CONbits.TMR1ON = 1;

    my_address = eeprom_read(0);
    if (my_address == 0xFF || my_address == 0) my_address = 0x01;

    /* инициализация синхронизации приёмника */
    prev_ticks      = TMR1;
    PIR1bits.TMR1IF = 0;
    bit_count       = 12;        /* ждём первую паузу, чтобы поймать начало кадра */

    DATA_IN_SetInterruptHandler(protocol_on_bus_change);
    INTCONbits.PEIE = 1;
    INTCONbits.GIE  = 1;

    G_LED_SetHigh(); __delay_ms(100); G_LED_SetLow();

    uint16_t idle_loops = 0;

    while (1) {
        if (cmd_ready) {
            cmd_ready = 0;
            process_command();
            idle_loops = 0;                 /* любой кадр на шине = активность */
        } else if (++idle_loops >= IDLE_RESET_LOOPS) {
            idle_loops = 0;
            reset_sequencers();             /* шина молчит — сбрасываем недобранные последовательности */
        }

        /* Выдача фаз ответа по меткам T2 (флаг ставит ISR). */
		if (in_response && resp_request) {
			resp_request = 0;

			/* Глушим вход на время передачи: собственные импульсы ответа идут по той же
			 * шине и иначе возвращаются в ISR, ложно взводя resp_request/границу кадра. */
			IOCBP = 0x0;
			IOCBN = 0x0;

			send_response_phase(resp_phase);

			prev_ticks       = TMR1;   /* опорная точка заново — чтобы пауза не сошла за GAP */
			IOCBF            = 0x0;     /* сбросить флаги, накопленные за передачу           */
			PIR1bits.TMR1IF  = 0;
			resp_request     = 0;       /* и фантомный запрос от собственного фронта          */
			IOCBP = 0x1;
			IOCBN = 0x1;

			if (++resp_phase >= 3) {
				in_response = 0;
				resp_armed  = 0;
				resp_phase  = 0;
			}
		}

        __delay_us(100);
    }
}
