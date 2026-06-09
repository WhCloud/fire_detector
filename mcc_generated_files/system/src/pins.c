/**
 * Generated Driver File
 * 
 * @file pins.c
 * 
 * @ingroup  pinsdriver
 * 
 * @brief This is generated driver implementation for pins. 
 *        This file provides implementations for pin APIs for all pins selected in the GUI.
 *
 * @version Driver Version 3.0.0
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

#include "../pins.h"

void (*DATA_IN_InterruptHandler)(void);

void PIN_MANAGER_Initialize(void)
{
   /**
    LATx registers
    */
    LATA = 0x0;
    LATB = 0x0;
    LATC = 0x0;
    LATE = 0x8;

    /**
    TRISx registers
    */
    TRISA = 0xF7;
    TRISB = 0x7F;
    TRISC = 0xF;
    TRISE = 0x8;

    /**
    ANSELx registers
    */
    ANSELA = 0x37;
    ANSELB = 0x16;

    /**
    WPUx registers
    */
    WPUB = 0x16;
    WPUE = 0x8;
    OPTION_REGbits.nWPUEN = 0x0;
  
    /**
    ODx registers
    */
   
    /**
    SLRCONx registers
    */
    /**
    INLVLx registers
    */

    /**
    PPS registers
    */

    /**
    APFCON registers
    */

   /**
    IOCx registers 
    */
    IOCBP = 0x1;
    IOCBN = 0x1;
    IOCBF = 0x0;

    DATA_IN_SetInterruptHandler(DATA_IN_DefaultInterruptHandler);

    // Enable INTCONbits.IOCIE interrupt 
    INTCONbits.IOCIE = 1; 
}
  
void PIN_MANAGER_IOC(void)
{
    // interrupt on change for pin DATA_IN}
    if(IOCBFbits.IOCBF0 == 1)
    {
        DATA_IN_ISR();  
    }
}
   
/**
   DATA_IN Interrupt Service Routine
*/
void DATA_IN_ISR(void) {

    // Add custom IOCBF0 code

    // Call the interrupt handler for the callback registered at runtime
    if(DATA_IN_InterruptHandler)
    {
        DATA_IN_InterruptHandler();
    }
    IOCBFbits.IOCBF0 = 0;
}

/**
  Allows selecting an interrupt handler for IOCBF0 at application runtime
*/
void DATA_IN_SetInterruptHandler(void (* InterruptHandler)(void)){
    DATA_IN_InterruptHandler = InterruptHandler;
}

/**
  Default interrupt handler for IOCBF0
*/
void DATA_IN_DefaultInterruptHandler(void){
    // add your DATA_IN interrupt custom code
    // or set custom function using DATA_IN_SetInterruptHandler()
}
/**
 End of File
*/