/*********************************************************************************

        @file   System_INIT_OPS.c
        @author P. Schlein, Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU - system initialization
        @note   
        @version 1.1
        @copyright 2011 P. Schlein, Jon Zeeff

*************************************************************************************/

/* 
Portions Copyright 2011 P. Schlein - MIT License
Portions Copyright 2011,2012  Jon Zeeff - All rights reserved
*/

#include <stdint.h>
#include "config.h"
#include "mpc563xm.h"
#include "System_INIT_OPS.h"
#include "eQADC_OPS.h"
#include "eMIOS_OPS.h"
#include "etpu_util.h"
#include "SIU_OPS.h"
#include "eDMA_OPS.h"
#include "eSCI_OPS.h"
#include "eSCI_DMA.h"
#include "FLASH_OPS.h"
#include "variables.h"
#include "eTPU_OPS.h"
#include "led.h"

#define PartNumber SIU.MIDR.B.PARTNUM    // CPU ID - is in hex, example: 0x5634

/**************************************************************************
     * FUNCTION : SYS_INIT 
     * PURPOSE : This function intializes the OPENECU System 
     * INPUT NOTES : none 
     * RETURN NOTES : None 
     * WARNING : None 
**************************************************************************/
void System_Init(void)
{
    led_set( 1 );
    // freeze here if wrong CPU
    if (PartNumber != 0x5634)
        for (;;) {
        }

    // Initialize Flash and cpu speed
    led_set( 2 );
//    init_FLASH();  /**< already done by bsp on startup */

    // Interrupts setup 
    led_set( 4 );
    //INTC_InitINTCInterrupts();  /**< already done by bsp on startup */

    // Timer tick ISR for OS - See RM for source of 51 (first eMIOS interrupt vector)
    //INTC_InstallINTCInterruptHandler((INTCInterruptFn) msec_ISR, 51 + MSEC_TIMER_EMIOS_CHANNEL,0);

    // other interrupts can be set up here

    // Initialize variables (checks if flash is loaded)
    led_set( 5 );
    init_variables(); /**< touches Flash_OK global */

    // Initialize OS
    led_set( 6 );

    if (Flash_OK) {

    // Initialize eTPU 
    led_set( 7 );
    (void)init_eTPU();
    //init_PWM1(200); // uses eTPU
    //Init_Tach();    // uses eTPU

    // Enable all timebases-move to eTPU_OPS.c later
    led_set( 8 );
    fs_timer_start();

    // Initialize ADC
    led_set( 9 );
    init_ADC();         // DMA version

    // Initialize eDMA for AD
    led_set( 10 );
    init_eDMA();

    // Initialize Watchdog 
    led_set( 11 );

    } // if

    // Initialize SIU (pin functions)
    led_set( 12 );
    init_SIU();

    // Initialize eMIOS (timers/clocks)
    led_set( 13 );
    init_eMIOS();

    // Initialize eSCI (serial ports)
    led_set( 14 );               // careful, overwrites what SIU did
    init_eSCI();
    init_eSCI_DMA();

    // Initialize SPI
    led_set( 0 );
    // Init_SPI();

}