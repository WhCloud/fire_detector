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
 *   Граница кадра определяется ТОЛЬКО по измеренному интервалу:
 *       gap = (delta >= GAP_TICKS)
 *   16-битное вычитание (cur - prev_ticks) корректно меряет интервал ДАЖЕ когда
 *   TMR1 перешёл через 0xFFFF. Прескалер 1:8 (2 мкс/тик) даёт диапазон 131 мс ≫
 *   паузы 16 мс (8000 тиков), поэтому реальные интервалы НИКОГДА не заворачиваются
 *   и delta всегда достоверна. Флаг TMR1IF НЕ используется как граница: он значит
 *   «таймер перешёл ноль», а не «прошло много времени»; обычный бит-интервал,
 *   попавший на 0xFFFF->0, ложно сбрасывал приём -> 1 пропуск кадра с периодом
 *   ~131 мс (тот самый редкий пропуск среди долгих серий удачных посылок).
 *   Порог GAP_TICKS лежит между макс. внутрикадровым интервалом (окно ответа ~2.5 мс)
 *   и минимальной паузой между кадрами (~7..16 мс).
 */

/* Поменяны названия светодиодов. везде после смены названия поставлен коммент //ch old_RYG (какой был) */

#include "mcc_generated_files/system/system.h"
#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 16000000

#define MODULE_TYPE 0x06
#define CURRENT_WIDTH_LOW 600
#define CURRENT_WIDTH_HIGH 2000
#define ADC_REF_MVOLTS 5000
#define ADC_VALUE_HIGH 1000
#define ADC_VALUE_LOW 300

/* Цифровые входы расширения (PIC16F1936, 28 pin):
 *  Пины 3 (RA1) и 4 (RA2) — «разрешение»: если хотя бы один = 0, игнор АЦП и шлём
 *    ошибку (нулевой импульс smoke/heat). ANSEL для них сбрасывается в main()
 *    (MCC ставит RA1/RA2 аналоговыми) — иначе цифровое чтение вернёт всегда 0.
 *  Пины 24 (RB3=MODE1) и 26 (RB5=MODE2) — 2-битный селектор режима:
 *    бит1 = пин 24 (RB3), бит0 = пин 26 (RB5); уже цифровые входы.
 *  Пины 22 (RB1=AN10) и 23 (RB2=AN8) — аналоговые входы АЦП (см. ADC_CH_* ниже). */
#define ENABLE_PIN3_GetValue()  PORTAbits.RA1
#define ENABLE_PIN4_GetValue()  PORTAbits.RA2

/* Каналы АЦП (PIC16F1936, 28 pin):
 *  Пин 2  (RA0) = AN0  — основной вход (был единственным).
 *  Пин 22 (RB1) = AN10 — второй вход: меряется так же, на линию идёт БОЛЬШЕЕ из двух.
 *  Пин 23 (RB2) = AN8  — инициализирован, в ответе пока не участвует.
 * ANSELB/WPUB для 22/23 доводятся в main() (MCC оставляет подтяжки включёнными). */
#define ADC_CH_MAIN    0u   /* AN0,  RA0, пин 2  */
#define ADC_CH_LINE  10u   /* AN10, RB1, пин 22 */
#define ADC_CH_CURR   8u   /* AN8,  RB2, пин 23 */

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
uint16_t          resp_pull_us = CURRENT_WIDTH_LOW; /* предчитанная ширина тока Smoke/Heat */

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

/* Чтение 16-битного TMR1 без «разрыва»: если младший байт перескочил во время
 * чтения, старший мог измениться — перечитываем (ошибка на 256 тиков = 512 мкс
 * при 1:8 перепутала бы 0/1 бита). Общий для ISR и send_pulse. */
static uint16_t tmr1_ticks(void) {
    uint8_t hi = TMR1H;
    uint8_t lo = TMR1L;
    if (TMR1H != hi) { hi = TMR1H; lo = TMR1L; }
    return ((uint16_t)hi << 8) | lo;
}

/* =========================================================================
 *  Приёмник кадров — обработчик прерывания по фронту на DATA_IN (RB0)
 *  Никаких блокирующих задержек: они сбивают измерение следующего интервала.
 * ========================================================================= */
void protocol_on_bus_change(void) {
    uint16_t cur = tmr1_ticks();   /* glitch-free чтение TMR1 (см. выше) */

    PIR1bits.TMR1IF = 0;     /* чистим флаг, но как границу кадра НЕ используем */

    uint16_t delta = (uint16_t)(cur - prev_ticks);   /* 16-битная арифметика с заворотом */
    prev_ticks = cur;

    /* --- Граница кадра: длинная пауза шины (ТОЛЬКО по delta) ---
     * 16-битное вычитание уже корректно меряет интервал через переворот TMR1.
     * Флаг переполнения как граница НЕ годится: TMR1IF=1 значит «таймер перешёл
     * 0xFFFF->0», а НЕ «прошло много времени». Обычный бит-интервал, попавший на
     * переворот, ложно взводил флаг -> сброс bit_count в середине кадра -> ровно
     * 1 пропуск кадра с периодом переполнения (~131 мс). Поэтому ovf убран. */
    if (delta >= GAP_TICKS) {
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
 *
 *  Ответ идёт по ТОЙ ЖЕ шине, что и приём: наш pull-back ток виден на RB0.
 *  Поэтому на время КАЖДОГО импульса IOC глушится снаружи (см. главный цикл),
 *  а фазы выдаются СТРОГО по меткам T2 — одна метка = одна фаза.
 * ========================================================================= */

/* Импульс тока заданной ширины, отмеренный по TMR1 (2 мкс/тик, уже запущен).
 * Цикл __delay_us на -O0 добавлял оверхед на итерацию -> 808 мкс выходило ~865.
 * TMR1: ошибка ±1 тик (~2 мкс) и НЕ накапливается с длиной импульса. IOC на время
 * импульса заглушён снаружи + других прерываний нет -> цикл не прерывается. Чтение
 * TMR1 на приём не влияет (prev_ticks правит только ISR). us чётное (=2600-2*mV). */
void send_pulse(uint16_t us) {
    if (us > 0) { 
        uint16_t ticks = US_TO_TICKS(us);
        DATA_OUT_SetHigh();
        uint16_t t0 = tmr1_ticks();
        while ((uint16_t)(tmr1_ticks() - t0) < ticks) { }
        DATA_OUT_SetLow();
    } else {
        /* Не посылать ничего в случае ошибки */
    }
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

/* Одно измерение указанного канала. 10 мкс на заряд C_HOLD после смены канала
 * (Tacq по спеке ~5 мкс) обязательны: иначе первое измерение после переключения
 * тянет остаток заряда от предыдущего канала. */
uint16_t read_adc_ch(uint8_t chs) {
    ADCON1 = 0xA0;   /* ADFM=1 (right), ADCS=Fosc/32 (Tad=2мкс @16МГц; Fosc/2 был вне спеки 1мкс), ADPREF=VDD */
    ADCON0 = 0x01;
    ADCON0bits.CHS = chs;
    __delay_us(10);
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    return ((uint16_t)ADRESH << 8) | ADRESL;
}

/* Меряем оба входа, на линию идёт БОЛЬШЕЕ напряжение. Сравниваем сырые коды АЦП:
 * шкала у каналов общая (VDD/1024), поэтому больший код = большее напряжение,
 * а конверсия в мВ нужна ровно одна. */
uint16_t read_adc_max(void) {
    uint16_t v_main  = read_adc_ch(ADC_CH_MAIN);
    uint16_t v_pin22 = read_adc_ch(ADC_CH_LINE);
    return (v_pin22 > v_main) ? v_pin22 : v_main;
}

/* Одна фаза ответа по метке T2. Значение тока (resp_pull_us) предчитано на старте
 * ответа -> здесь только блокирующий импульс, минимум задержки метка->импульс. */
void send_response_phase(uint8_t phase) {
    switch (phase) {
        case 0: send_pulse(resp_pull_us);    break;   /* Smoke */
        case 1: send_type_bits(MODULE_TYPE); break;   /* Type  */
        case 2: send_pulse(resp_pull_us);    break;   /* Heat  */
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

static uint16_t convert_adc_to_pulse_usec(uint16_t adc_val) {
    uint16_t _pulse_width = 0;
    
    /* УМНОЖАТЬ до деления и в 32 битах. Иначе ADC_REF_MVOLTS/1024 = 5000/1024 = 4
     * (целочисленно, не 4.88) -> шкала занижена ~18%: 300мВ читается ~244мВ и
     * вылетает из окна -> импульс 0; ширины тоже врут. (5000*1023 > uint16.) */
    uint16_t adc_val_mvolts = (uint16_t)(((uint32_t)ADC_REF_MVOLTS * adc_val + 512u) / 1024u);
    if ((adc_val_mvolts >= ADC_VALUE_LOW) && (adc_val_mvolts <= ADC_VALUE_HIGH)) {
        _pulse_width = (CURRENT_WIDTH_HIGH + CURRENT_WIDTH_LOW) - 2 * adc_val_mvolts;
    }
    
    return _pulse_width;
}

/* Селектор режима на пинах 24 (RB3=MODE1, бит1) и 26 (RB5=MODE2, бит0).
 * 4 состояния (0..3). Заготовка: действий пока нет — заполнять по мере надобности
 * (напр. мигание светодиодом). Пины уже цифровые входы (ANSELB бит3/5 = 0). */
static void apply_mode_select(void) {
    uint8_t sel = (uint8_t)((MODE1_GetValue() << 1) | MODE2_GetValue());
    switch (sel) {
        case 0: {
                LOAD_SetHigh();
                G_LED_SetHigh(); //ch old_Y
            } 
            break;
        case 1: {
                LOAD_SetHigh();
                G_LED_SetHigh(); //ch old_Y
            }
            break;
        case 2: {
                LOAD_SetHigh();
                G_LED_SetHigh(); //ch old_Y
            }
            break;
        case 3: {
                LOAD_SetHigh();
                G_LED_SetHigh(); //ch old_Y
            }
            break;
        default: 
            break;
    }
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
                    /* НИКАКИХ блокирующих задержек здесь. Панель через ~46 мс шлёт
                     * Mode 0 на новый адрес для проверки записи; старая подтверждающая
                     * мигалка (4x __delay_ms(100) = 400 мс) держала main и съедала этот
                     * опрос -> устройство «не отвечало» после установки адреса.
                     * EEPROM-запись на 16F1936 идёт в фоне (~4 мс), CPU не стопит,
                     * а ответ использует my_address из RAM (выставлен выше). */
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
            /* Пины 3 (RA1) или 4 (RA2) в логическом 0 -> игнор АЦП, шлём ошибку:
             * нулевой импульс smoke/heat (Type не трогаем). Красный LED (RC4) —
             * отладочный индикатор ошибки: вкл при ошибке, выкл когда пропала. */
            if ((ENABLE_PIN3_GetValue() == 0) || (ENABLE_PIN4_GetValue() == 0)) {
                resp_pull_us = 0;
                G_LED_SetHigh(); //ch old_R
            } else {
                /* Больший из двух входов (пин 2 / пин 22) -> ширина импульса.
                 * Вне окна 300..1000 мВ conversion даёт 0 -> ответ не шлётся. */
                resp_pull_us = convert_adc_to_pulse_usec(read_adc_max());
                G_LED_SetLow(); //ch old_R
            }
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
                apply_mode_select(); // Если включена эта функция, то при работе ADC_CH_MAIN включается LOAD_SetHigh()!!!  
            }
            break;

        case 5:                      /* Calibrate Heat / Test / Calibrate Smoke */
        case 6:
        case 7:
            reset_counter    = 0;
            activate_counter = 0;
            /* короткая индикация без длинной блокировки шины */
            R_LED_SetHigh(); __delay_ms(60); R_LED_SetLow(); //ch old_G
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

    /* Пины 3 (RA1) и 4 (RA2): MCC ставит их аналоговыми (ANSELA=0x37) — для чтения
     * ЛОГИЧЕСКОГО уровня переводим в цифровой режим. TRISA уже вход (0xF7). АЦП
     * читает только AN0/RA0, эти каналы не нужны. */
    ANSELAbits.ANSA1 = 0;
    ANSELAbits.ANSA2 = 0;

    /* Пины 22 (RB1=AN10) и 23 (RB2=AN8) — аналоговые входы АЦП. ANSELB/TRISB MCC
     * уже ставит верно (0x16 / вход), но пишем явно, чтобы перегенерация не сломала.
     * WPUB=0x16 (pins.c) при nWPUEN=0 включает слабые подтяжки к VDD: на аналоговом
     * входе они тянут узел вверх и завышают измерение — снимаем. */
    ANSELBbits.ANSB1 = 1;
    ANSELBbits.ANSB2 = 1;
    TRISBbits.TRISB1 = 1;
    TRISBbits.TRISB2 = 1;
    WPUBbits.WPUB1   = 0;
    WPUBbits.WPUB2   = 0;

    /* стартовая индикация */
    for (uint8_t i = 0; i < 3; i++) {
        R_LED_SetHigh(); // ch old_G Мне надо, что бы при подаче питания без опроса просто горел индикатор
        /*__delay_ms(150);
        G_LED_SetLow();  __delay_ms(150);*/
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

    //G_LED_SetHigh(); __delay_ms(100); G_LED_SetLow(); //Вообще не понял зачем. Отключил.

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

        /* Одна метка T2 -> ОДНА фаза, строго ПОСЛЕ обнаружения метки (ISR ставит
         * resp_request). Heat уходит только после 3-й метки T2. */
        if (in_response && resp_request) {
            resp_request = 0;

            /* Глушим вход ТОЛЬКО на время своего импульса: ток ответа виден на RB0.
             * prev_ticks НЕ ресинкаем — следующая метка T2 ловится по своим краям;
             * resync ставил опору в середину окна, и край окна читался как метка. */
            IOCBP = 0x0;
            IOCBN = 0x0;
            send_response_phase(resp_phase);
            IOCBF = 0x0;            /* съесть флаги собственных фронтов */
            IOCBP = 0x1;
            IOCBN = 0x1;

            if (++resp_phase >= 3) {   /* выдали Smoke+Type+Heat -> ответ завершён */
                in_response = 0;
                resp_armed  = 0;
                resp_phase  = 0;
                R_LED_SetHigh(); __delay_ms(20); R_LED_SetLow(); //ch Раньше не было. Индикация завершившегося опроса
            }
        }

        if (!in_response) {
            __delay_us(100);                 /* в ответе — минимум задержки метка->импульс */
        }
    }
}
