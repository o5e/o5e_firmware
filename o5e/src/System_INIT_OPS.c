/**************************************************************************
 FILE NAME: System_INIT_OPS.c 

 DESCRIPTION: 
     This file contains functions for Initializing OPENECU 
     System dependant code. 

       ORIGINAL AUTHOR: Paul Schlein 
       REV  AUTHOR      DATE      	DESCRIPTION OF CHANGE 
       --- ----------- ----------       --------------------- 
       3.1 J. Zeeff    1/Oct/11 	Add OS and basics for interrrupts 
       3.0 P. Schlein  4/Sep/11 	Add Fuel Pump Init 
       2.0 P. Schlein 28/Jul/11 	Add eDMA anf eMIOSFunctions 
       1.0 P. Schlein 26/May/11 	Initial version 

**************************************************************************/

#include "config.h"
#include "system.h"
#include "SIU_OPS.h"
#include "IntcInterrupts.h"
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
#include "OS.h"
#include "Functions.h"

#if 0
// Interrupt service routine to increment OS clock
// Not used

void msec_ISR(void)
{
    os_task_tick();             // call routine to inc OS timer
    EMIOS.CH[MSEC_TIMER_EMIOS_CHANNEL].CADR.R = 132;    // reload counter
    EMIOS.CH[MSEC_TIMER_EMIOS_CHANNEL].CSR.B.FLAG = 1;  // Clear channel's flag 
}
#endif

/**************************************************************************
     * FUNCTION : SYS_INIT 
     * PURPOSE : This function intializes the OPENECU System 
     * INPUT NOTES : none 
     * RETURN NOTES : None 
     * WARNING : None 
**************************************************************************/
void System_Init(void)
{

    set_LEDs(1);
    // freeze here if wrong CPU
#ifdef MPC5634
    if (PartNumber != 0x5634)
        for (;;) {
        }
#endif

    // Initialize Flash and cpu speed
    set_LEDs(2);
    init_FLASH();

    // Interrupts setup 
    set_LEDs(4);
    INTC_InitINTCInterrupts();

    // Timer tick ISR for OS - See RM for source of 51 (first eMIOS interrupt vector)
    //INTC_InstallINTCInterruptHandler((INTCInterruptFn) msec_ISR, 51 + MSEC_TIMER_EMIOS_CHANNEL,0);

    // other interrupts can be set up here

    // Initialize variables (checks if flash is loaded)
    set_LEDs(5);
    init_variables();

    // Initialize OS
    set_LEDs(6);
    os_init();

    if (Flash_OK) {

    // Enable all timebases-move to eTPU_OPS.c later
    set_LEDs(7);
    fs_timer_start();

    // Initialize eTPU 
    set_LEDs(8);
    (void)init_eTPU();
    //init_PWM1(200); // uses eTPU
    //Init_Tach();    // uses eTPU

    // Initialize ADC
    set_LEDs(9);
    init_ADC();         // DMA version

    // Initialize eDMA for AD
    set_LEDs(10);
    init_eDMA();

    // Initialize SIU (pin functions)
    set_LEDs(11);
    init_SIU();

    // Initialize Watchdog 
    set_LEDs(12);
    Init_Watchdog();

    } // if

    // Initialize eMIOS (timers/clocks)
    set_LEDs(13);
    init_eMIOS();

    // Initialize eSCI (serial ports)
    set_LEDs(14);               // careful, overwrites what SIU did
    init_eSCI();

    // Initialize eSCI DMA
    set_LEDs(15);
    init_eSCI_DMA();

}

// diagnostic routine to set LEDs to binary code to see progress through the init routines
#include "FreeScale/FSutil.h"

// Note: this routine may over write pin settings set in SIU_OPS

void set_LEDs(int n)
{
#ifdef MPC5634
#   define LED1 188
#   define LED2 189
#   define LED3 190
#   define LED4 191

    init_LED(LED4, (n & 0x1) == 0);
    init_LED(LED3, (n & 0x2) == 0);
    init_LED(LED2, (n & 0x4) == 0);
    init_LED(LED1, (n & 0x8) == 0);
#endif

}
