/**
 * Generated Pins header File
 * 
 * @file pins.h
 * 
 * @defgroup  pinsdriver Pins Driver
 * 
 * @brief This is generated driver header for pins. 
 *        This header file provides APIs for all pins selected in the GUI.
 *
 * @version Driver Version  3.0.0
*/

/*
© [2026] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

#ifndef PINS_H
#define PINS_H

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set IO_RA0 aliases
#define _TRIS                 TRISAbits.TRISA0
#define _LAT                  LATAbits.LATA0
#define _PORT                 PORTAbits.RA0
#define _WPU                  WPUAbits.
#define _OD                   ODCONAbits.
#define _ANS                  ANSELAbits.ANSA0
#define _SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define _SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define _Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define _GetValue()           PORTAbits.RA0
#define _SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define _SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define _SetPullup()          do { WPUAbits. = 1; } while(0)
#define _ResetPullup()        do { WPUAbits. = 0; } while(0)
#define _SetPushPull()        do { ODCONAbits. = 0; } while(0)
#define _SetOpenDrain()       do { ODCONAbits. = 1; } while(0)
#define _SetAnalogMode()      do { ANSELAbits.ANSA0 = 1; } while(0)
#define _SetDigitalMode()     do { ANSELAbits.ANSA0 = 0; } while(0)
// get/set IO_RA1 aliases
#define EN_PIN3_TRIS                 TRISAbits.TRISA1
#define EN_PIN3_LAT                  LATAbits.LATA1
#define EN_PIN3_PORT                 PORTAbits.RA1
#define EN_PIN3_WPU                  WPUAbits.
#define EN_PIN3_OD                   ODCONAbits.
#define EN_PIN3_ANS                  ANSELAbits.ANSA1
#define EN_PIN3_SetHigh()            do { LATAbits.LATA1 = 1; } while(0)
#define EN_PIN3_SetLow()             do { LATAbits.LATA1 = 0; } while(0)
#define EN_PIN3_Toggle()             do { LATAbits.LATA1 = ~LATAbits.LATA1; } while(0)
#define EN_PIN3_GetValue()           PORTAbits.RA1
#define EN_PIN3_SetDigitalInput()    do { TRISAbits.TRISA1 = 1; } while(0)
#define EN_PIN3_SetDigitalOutput()   do { TRISAbits.TRISA1 = 0; } while(0)
#define EN_PIN3_SetPullup()          do { WPUAbits. = 1; } while(0)
#define EN_PIN3_ResetPullup()        do { WPUAbits. = 0; } while(0)
#define EN_PIN3_SetPushPull()        do { ODCONAbits. = 0; } while(0)
#define EN_PIN3_SetOpenDrain()       do { ODCONAbits. = 1; } while(0)
#define EN_PIN3_SetAnalogMode()      do { ANSELAbits.ANSA1 = 1; } while(0)
#define EN_PIN3_SetDigitalMode()     do { ANSELAbits.ANSA1 = 0; } while(0)
// get/set IO_RA2 aliases
#define EN_PIN4_TRIS                 TRISAbits.TRISA2
#define EN_PIN4_LAT                  LATAbits.LATA2
#define EN_PIN4_PORT                 PORTAbits.RA2
#define EN_PIN4_WPU                  WPUAbits.
#define EN_PIN4_OD                   ODCONAbits.
#define EN_PIN4_ANS                  ANSELAbits.ANSA2
#define EN_PIN4_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define EN_PIN4_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define EN_PIN4_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define EN_PIN4_GetValue()           PORTAbits.RA2
#define EN_PIN4_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define EN_PIN4_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define EN_PIN4_SetPullup()          do { WPUAbits. = 1; } while(0)
#define EN_PIN4_ResetPullup()        do { WPUAbits. = 0; } while(0)
#define EN_PIN4_SetPushPull()        do { ODCONAbits. = 0; } while(0)
#define EN_PIN4_SetOpenDrain()       do { ODCONAbits. = 1; } while(0)
#define EN_PIN4_SetAnalogMode()      do { ANSELAbits.ANSA2 = 1; } while(0)
#define EN_PIN4_SetDigitalMode()     do { ANSELAbits.ANSA2 = 0; } while(0)
// get/set IO_RA3 aliases
#define DATA_OUT_TRIS                 TRISAbits.TRISA3
#define DATA_OUT_LAT                  LATAbits.LATA3
#define DATA_OUT_PORT                 PORTAbits.RA3
#define DATA_OUT_WPU                  WPUAbits.
#define DATA_OUT_OD                   ODCONAbits.
#define DATA_OUT_ANS                  ANSELAbits.ANSA3
#define DATA_OUT_SetHigh()            do { LATAbits.LATA3 = 1; } while(0)
#define DATA_OUT_SetLow()             do { LATAbits.LATA3 = 0; } while(0)
#define DATA_OUT_Toggle()             do { LATAbits.LATA3 = ~LATAbits.LATA3; } while(0)
#define DATA_OUT_GetValue()           PORTAbits.RA3
#define DATA_OUT_SetDigitalInput()    do { TRISAbits.TRISA3 = 1; } while(0)
#define DATA_OUT_SetDigitalOutput()   do { TRISAbits.TRISA3 = 0; } while(0)
#define DATA_OUT_SetPullup()          do { WPUAbits. = 1; } while(0)
#define DATA_OUT_ResetPullup()        do { WPUAbits. = 0; } while(0)
#define DATA_OUT_SetPushPull()        do { ODCONAbits. = 0; } while(0)
#define DATA_OUT_SetOpenDrain()       do { ODCONAbits. = 1; } while(0)
#define DATA_OUT_SetAnalogMode()      do { ANSELAbits.ANSA3 = 1; } while(0)
#define DATA_OUT_SetDigitalMode()     do { ANSELAbits.ANSA3 = 0; } while(0)
// get/set IO_RB0 aliases
#define DATA_IN_TRIS                 TRISBbits.TRISB0
#define DATA_IN_LAT                  LATBbits.LATB0
#define DATA_IN_PORT                 PORTBbits.RB0
#define DATA_IN_WPU                  WPUBbits.WPUB0
#define DATA_IN_OD                   ODCONBbits.
#define DATA_IN_ANS                  ANSELBbits.ANSB0
#define DATA_IN_SetHigh()            do { LATBbits.LATB0 = 1; } while(0)
#define DATA_IN_SetLow()             do { LATBbits.LATB0 = 0; } while(0)
#define DATA_IN_Toggle()             do { LATBbits.LATB0 = ~LATBbits.LATB0; } while(0)
#define DATA_IN_GetValue()           PORTBbits.RB0
#define DATA_IN_SetDigitalInput()    do { TRISBbits.TRISB0 = 1; } while(0)
#define DATA_IN_SetDigitalOutput()   do { TRISBbits.TRISB0 = 0; } while(0)
#define DATA_IN_SetPullup()          do { WPUBbits.WPUB0 = 1; } while(0)
#define DATA_IN_ResetPullup()        do { WPUBbits.WPUB0 = 0; } while(0)
#define DATA_IN_SetPushPull()        do { ODCONBbits. = 0; } while(0)
#define DATA_IN_SetOpenDrain()       do { ODCONBbits. = 1; } while(0)
#define DATA_IN_SetAnalogMode()      do { ANSELBbits.ANSB0 = 1; } while(0)
#define DATA_IN_SetDigitalMode()     do { ANSELBbits.ANSB0 = 0; } while(0)
#define RB0_SetInterruptHandler  DATA_IN_SetInterruptHandler
// get/set IO_RB3 aliases
#define MODE1_TRIS                 TRISBbits.TRISB3
#define MODE1_LAT                  LATBbits.LATB3
#define MODE1_PORT                 PORTBbits.RB3
#define MODE1_WPU                  WPUBbits.WPUB3
#define MODE1_OD                   ODCONBbits.
#define MODE1_ANS                  ANSELBbits.ANSB3
#define MODE1_SetHigh()            do { LATBbits.LATB3 = 1; } while(0)
#define MODE1_SetLow()             do { LATBbits.LATB3 = 0; } while(0)
#define MODE1_Toggle()             do { LATBbits.LATB3 = ~LATBbits.LATB3; } while(0)
#define MODE1_GetValue()           PORTBbits.RB3
#define MODE1_SetDigitalInput()    do { TRISBbits.TRISB3 = 1; } while(0)
#define MODE1_SetDigitalOutput()   do { TRISBbits.TRISB3 = 0; } while(0)
#define MODE1_SetPullup()          do { WPUBbits.WPUB3 = 1; } while(0)
#define MODE1_ResetPullup()        do { WPUBbits.WPUB3 = 0; } while(0)
#define MODE1_SetPushPull()        do { ODCONBbits. = 0; } while(0)
#define MODE1_SetOpenDrain()       do { ODCONBbits. = 1; } while(0)
#define MODE1_SetAnalogMode()      do { ANSELBbits.ANSB3 = 1; } while(0)
#define MODE1_SetDigitalMode()     do { ANSELBbits.ANSB3 = 0; } while(0)
// get/set IO_RB5 aliases
#define MODE2_TRIS                 TRISBbits.TRISB5
#define MODE2_LAT                  LATBbits.LATB5
#define MODE2_PORT                 PORTBbits.RB5
#define MODE2_WPU                  WPUBbits.WPUB5
#define MODE2_OD                   ODCONBbits.
#define MODE2_ANS                  ANSELBbits.ANSB5
#define MODE2_SetHigh()            do { LATBbits.LATB5 = 1; } while(0)
#define MODE2_SetLow()             do { LATBbits.LATB5 = 0; } while(0)
#define MODE2_Toggle()             do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define MODE2_GetValue()           PORTBbits.RB5
#define MODE2_SetDigitalInput()    do { TRISBbits.TRISB5 = 1; } while(0)
#define MODE2_SetDigitalOutput()   do { TRISBbits.TRISB5 = 0; } while(0)
#define MODE2_SetPullup()          do { WPUBbits.WPUB5 = 1; } while(0)
#define MODE2_ResetPullup()        do { WPUBbits.WPUB5 = 0; } while(0)
#define MODE2_SetPushPull()        do { ODCONBbits. = 0; } while(0)
#define MODE2_SetOpenDrain()       do { ODCONBbits. = 1; } while(0)
#define MODE2_SetAnalogMode()      do { ANSELBbits.ANSB5 = 1; } while(0)
#define MODE2_SetDigitalMode()     do { ANSELBbits.ANSB5 = 0; } while(0)
// get/set IO_RB6 aliases
#define ICSPCLK_TRIS                 TRISBbits.TRISB6
#define ICSPCLK_LAT                  LATBbits.LATB6
#define ICSPCLK_PORT                 PORTBbits.RB6
#define ICSPCLK_WPU                  WPUBbits.WPUB6
#define ICSPCLK_OD                   ODCONBbits.
#define ICSPCLK_ANS                  ANSELBbits.
#define ICSPCLK_SetHigh()            do { LATBbits.LATB6 = 1; } while(0)
#define ICSPCLK_SetLow()             do { LATBbits.LATB6 = 0; } while(0)
#define ICSPCLK_Toggle()             do { LATBbits.LATB6 = ~LATBbits.LATB6; } while(0)
#define ICSPCLK_GetValue()           PORTBbits.RB6
#define ICSPCLK_SetDigitalInput()    do { TRISBbits.TRISB6 = 1; } while(0)
#define ICSPCLK_SetDigitalOutput()   do { TRISBbits.TRISB6 = 0; } while(0)
#define ICSPCLK_SetPullup()          do { WPUBbits.WPUB6 = 1; } while(0)
#define ICSPCLK_ResetPullup()        do { WPUBbits.WPUB6 = 0; } while(0)
#define ICSPCLK_SetPushPull()        do { ODCONBbits. = 0; } while(0)
#define ICSPCLK_SetOpenDrain()       do { ODCONBbits. = 1; } while(0)
#define ICSPCLK_SetAnalogMode()      do { ANSELBbits. = 1; } while(0)
#define ICSPCLK_SetDigitalMode()     do { ANSELBbits. = 0; } while(0)
// get/set IO_RB7 aliases
#define ICSPDAT_TRIS                 TRISBbits.TRISB7
#define ICSPDAT_LAT                  LATBbits.LATB7
#define ICSPDAT_PORT                 PORTBbits.RB7
#define ICSPDAT_WPU                  WPUBbits.WPUB7
#define ICSPDAT_OD                   ODCONBbits.
#define ICSPDAT_ANS                  ANSELBbits.
#define ICSPDAT_SetHigh()            do { LATBbits.LATB7 = 1; } while(0)
#define ICSPDAT_SetLow()             do { LATBbits.LATB7 = 0; } while(0)
#define ICSPDAT_Toggle()             do { LATBbits.LATB7 = ~LATBbits.LATB7; } while(0)
#define ICSPDAT_GetValue()           PORTBbits.RB7
#define ICSPDAT_SetDigitalInput()    do { TRISBbits.TRISB7 = 1; } while(0)
#define ICSPDAT_SetDigitalOutput()   do { TRISBbits.TRISB7 = 0; } while(0)
#define ICSPDAT_SetPullup()          do { WPUBbits.WPUB7 = 1; } while(0)
#define ICSPDAT_ResetPullup()        do { WPUBbits.WPUB7 = 0; } while(0)
#define ICSPDAT_SetPushPull()        do { ODCONBbits. = 0; } while(0)
#define ICSPDAT_SetOpenDrain()       do { ODCONBbits. = 1; } while(0)
#define ICSPDAT_SetAnalogMode()      do { ANSELBbits. = 1; } while(0)
#define ICSPDAT_SetDigitalMode()     do { ANSELBbits. = 0; } while(0)
// get/set IO_RC4 aliases
#define R_LED_TRIS                 TRISCbits.TRISC4
#define R_LED_LAT                  LATCbits.LATC4
#define R_LED_PORT                 PORTCbits.RC4
#define R_LED_WPU                  WPUCbits.
#define R_LED_OD                   ODCONCbits.
#define R_LED_ANS                  ANSELCbits.
#define R_LED_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define R_LED_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define R_LED_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define R_LED_GetValue()           PORTCbits.RC4
#define R_LED_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define R_LED_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define R_LED_SetPullup()          do { WPUCbits. = 1; } while(0)
#define R_LED_ResetPullup()        do { WPUCbits. = 0; } while(0)
#define R_LED_SetPushPull()        do { ODCONCbits. = 0; } while(0)
#define R_LED_SetOpenDrain()       do { ODCONCbits. = 1; } while(0)
#define R_LED_SetAnalogMode()      do { ANSELCbits. = 1; } while(0)
#define R_LED_SetDigitalMode()     do { ANSELCbits. = 0; } while(0)
// get/set IO_RC5 aliases
#define G_LED_TRIS                 TRISCbits.TRISC5
#define G_LED_LAT                  LATCbits.LATC5
#define G_LED_PORT                 PORTCbits.RC5
#define G_LED_WPU                  WPUCbits.
#define G_LED_OD                   ODCONCbits.
#define G_LED_ANS                  ANSELCbits.
#define G_LED_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define G_LED_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define G_LED_Toggle()             do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define G_LED_GetValue()           PORTCbits.RC5
#define G_LED_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define G_LED_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)
#define G_LED_SetPullup()          do { WPUCbits. = 1; } while(0)
#define G_LED_ResetPullup()        do { WPUCbits. = 0; } while(0)
#define G_LED_SetPushPull()        do { ODCONCbits. = 0; } while(0)
#define G_LED_SetOpenDrain()       do { ODCONCbits. = 1; } while(0)
#define G_LED_SetAnalogMode()      do { ANSELCbits. = 1; } while(0)
#define G_LED_SetDigitalMode()     do { ANSELCbits. = 0; } while(0)
// get/set IO_RC6 aliases
#define Y_LED_TRIS                 TRISCbits.TRISC6
#define Y_LED_LAT                  LATCbits.LATC6
#define Y_LED_PORT                 PORTCbits.RC6
#define Y_LED_WPU                  WPUCbits.
#define Y_LED_OD                   ODCONCbits.
#define Y_LED_ANS                  ANSELCbits.
#define Y_LED_SetHigh()            do { LATCbits.LATC6 = 1; } while(0)
#define Y_LED_SetLow()             do { LATCbits.LATC6 = 0; } while(0)
#define Y_LED_Toggle()             do { LATCbits.LATC6 = ~LATCbits.LATC6; } while(0)
#define Y_LED_GetValue()           PORTCbits.RC6
#define Y_LED_SetDigitalInput()    do { TRISCbits.TRISC6 = 1; } while(0)
#define Y_LED_SetDigitalOutput()   do { TRISCbits.TRISC6 = 0; } while(0)
#define Y_LED_SetPullup()          do { WPUCbits. = 1; } while(0)
#define Y_LED_ResetPullup()        do { WPUCbits. = 0; } while(0)
#define Y_LED_SetPushPull()        do { ODCONCbits. = 0; } while(0)
#define Y_LED_SetOpenDrain()       do { ODCONCbits. = 1; } while(0)
#define Y_LED_SetAnalogMode()      do { ANSELCbits. = 1; } while(0)
#define Y_LED_SetDigitalMode()     do { ANSELCbits. = 0; } while(0)
// get/set IO_RC7 aliases
#define LOAD_TRIS                 TRISCbits.TRISC7
#define LOAD_LAT                  LATCbits.LATC7
#define LOAD_PORT                 PORTCbits.RC7
#define LOAD_WPU                  WPUCbits.
#define LOAD_OD                   ODCONCbits.
#define LOAD_ANS                  ANSELCbits.
#define LOAD_SetHigh()            do { LATCbits.LATC7 = 1; } while(0)
#define LOAD_SetLow()             do { LATCbits.LATC7 = 0; } while(0)
#define LOAD_Toggle()             do { LATCbits.LATC7 = ~LATCbits.LATC7; } while(0)
#define LOAD_GetValue()           PORTCbits.RC7
#define LOAD_SetDigitalInput()    do { TRISCbits.TRISC7 = 1; } while(0)
#define LOAD_SetDigitalOutput()   do { TRISCbits.TRISC7 = 0; } while(0)
#define LOAD_SetPullup()          do { WPUCbits. = 1; } while(0)
#define LOAD_ResetPullup()        do { WPUCbits. = 0; } while(0)
#define LOAD_SetPushPull()        do { ODCONCbits. = 0; } while(0)
#define LOAD_SetOpenDrain()       do { ODCONCbits. = 1; } while(0)
#define LOAD_SetAnalogMode()      do { ANSELCbits. = 1; } while(0)
#define LOAD_SetDigitalMode()     do { ANSELCbits. = 0; } while(0)
/**
 * @ingroup  pinsdriver
 * @brief GPIO and peripheral I/O initialization
 * @param none
 * @return none
 */
void PIN_MANAGER_Initialize (void);

/**
 * @ingroup  pinsdriver
 * @brief Interrupt on Change Handling routine
 * @param none
 * @return none
 */
void PIN_MANAGER_IOC(void);

/**
 * @ingroup  pinsdriver
 * @brief Interrupt on Change Handler for the DATA_IN pin functionality
 * @param none
 * @return none
 */
void DATA_IN_ISR(void);

/**
 * @ingroup  pinsdriver
 * @brief Interrupt Handler Setter for DATA_IN pin interrupt-on-change functionality.
 *        Allows selecting an interrupt handler for DATA_IN at application runtime.
 * @pre Pins intializer called
 * @param InterruptHandler function pointer.
 * @return none
 */
void DATA_IN_SetInterruptHandler(void (* InterruptHandler)(void));

/**
 * @ingroup  pinsdriver
 * @brief Dynamic Interrupt Handler for DATA_IN pin.
 *        This is a dynamic interrupt handler to be used together with the DATA_IN_SetInterruptHandler() method.
 *        This handler is called every time the DATA_IN ISR is executed and allows any function to be registered at runtime.
 * @pre Pins intializer called
 * @param none
 * @return none
 */
extern void (*DATA_IN_InterruptHandler)(void);

/**
 * @ingroup  pinsdriver
 * @brief Default Interrupt Handler for DATA_IN pin. 
 *        This is a predefined interrupt handler to be used together with the DATA_IN_SetInterruptHandler() method.
 *        This handler is called every time the DATA_IN ISR is executed. 
 * @pre Pins intializer called
 * @param none
 * @return none
 */
void DATA_IN_DefaultInterruptHandler(void);


#endif // PINS_H
/**
 End of File
*/