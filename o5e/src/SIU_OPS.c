/******************************************************************************************/
/* FILE NAME: SIU_OPS.c                                                                   */
/*                                                                                        */
/* DESCRIPTION:                                                                           */
/* This file contains functions for the MPC5xxx to initialize the SIU                     */
/* Ie, what function each external pin is used for                                        */
/*                                                                                        */
/******************************************************************************************/

// Portions Copyright 2011 P. Schlein - MIT License
// Portions Copyright 2011,2012  M. Eberhardt - MIT License
// Portions Copyright 2011,2012  Jon Zeeff - All rights reserved

#include "config.h"
#include "mpc563xm.h"
#include "SIU_OPS.h"
#include "variables.h"

// PCR - Pad Configuration Register 
// OBE - Output Buffer Enable 
// IBE - Input Buffer Enable 
// ODE - Output Drive Enable 
// HYS - Hystreresis 
// SRC - Slew Rate Control 
// WPE - Weak Pull up Enable 
// WPS - Weak Pull up Select

// pin function bits PA
#define GPIO 		(0 << 10)       // 000  AF0
#define PRIMARY 	(1 << 10)       // 001  AF1
#define ALTERNATE 	(2 << 10)       // 010  AF2
#define MAIN_PRIMARY 	(3 << 10)       // 011  AF3
#define ALT2 		(4 << 10)       // 100  AF4
#define ALT3 		(8 << 10)       // 1000 AF8
// better method - since the RMs are so inconsistent
#define B0000  (0 << 10)   // always GPIO?
#define B0001  (1 << 10)
#define B0010  (2 << 10)   // some 3 or 4 function pins
#define B0011  (3 << 10)   // some 3 or 4 function pins 
#define B0100  (4 << 10)   // only 4 function pins
#define B1000  (8 << 10)

// in/out
#define UNUSED 	(0 << 8)
#define OUTPUT 	(1 << 9)
#define INPUT 	(1 << 8)

// weak pull up/down
#define WPE 	(1 << 1)
#define WPS 	(1 << 0)

// Mark - this is right?
#define CONFIG_INJ PRIMARY
#define CONFIG_IGN PRIMARY


/******************************************************************************************/
/* FUNCTION     : initSIU                                                                 */
/* PURPOSE      :   Setup MPC5554 Pads for inputs and Outputs                             */
/* INPUT NOTES  :   MPC5554EVB I/O Header Ring pins are someimes Id'd                     */
/*                  SIU PCR Ref.-see RM 6.3.1.12, 6-18, pg 202                            */
/* RETURN NOTES : None                                                                    */
/* WARNING      : Outputs generally set to 10PF                                           */
/******************************************************************************************/

// NOTE:  In my opinion, doing all the pad assignments here is a mistake and pad functions should be assigned
//        by the code that uses them.  For example, setting pin 100 to GPIO for use as radiator 
//        fan control should be done by the radiator fan init routine.

void init_SIU(void)
{
/*****************************************************************************************/

    // Note: AN12-AN15 have pad numbers and use B0011 to make them AN pins

        // eTPU0, pin 114 is special and needs a 11 instead of 01 for primary function
        SIU.PCR[114].R = B0011 | INPUT; 	// configure  for CRANK (TCRCLKA or eTPU[0])
        SIU.PCR[115].R = B0001 | INPUT;     // cam sensor input eTPU1
        SIU.PCR[116].R = B0001 | OUTPUT; 	// eTPU[2]  ignition 1
        SIU.PCR[117].R = B0001 | OUTPUT; 	// eTPU[3] ignition 2
        SIU.PCR[118].R = B0001 | OUTPUT; 	// eTPU[4] ignition 3
        SIU.PCR[119].R = B0001 | OUTPUT; 	// eTPU[5] ignition 4
        SIU.PCR[120].R = B0001 | OUTPUT; 	// eTPU[6] injector 1
        SIU.PCR[121].R = B0001 | OUTPUT; 	// eTPU[7] injector 2
        SIU.PCR[122].R = B0001 | OUTPUT; 	// eTPU[8] injector 3
        SIU.PCR[123].R = B0001 | OUTPUT; 	// eTPU[9] injector 4
        SIU.PCR[124].R = B0001 | OUTPUT; 	// eTPU[10] injector 5
        SIU.PCR[125].R = B0001 | OUTPUT; 	// eTPU[11] injector 6  - injector 12
        SIU.PCR[126].R = B0001 | OUTPUT; 	// eTPU[12] injector 7
        SIU.PCR[127].R = B0001 | OUTPUT; 	// eTPU[13] injector 8   - injector 11
        SIU.PCR[128].R = B0001 | OUTPUT; 	// eTPU[14]  
        SIU.PCR[129].R = B0001 | OUTPUT; 	// eTPU[15] 
        SIU.PCR[130].R = B0001 | OUTPUT;    // eTPU[16] 
        SIU.PCR[131].R = B0001 | OUTPUT;    // eTPU[17] 
        SIU.PCR[132].R = B0001 | UNUSED;    // eTPU[18]  
        SIU.PCR[133].R = B0001 | UNUSED;    // eTPU[19] 
        SIU.PCR[134].R = B0001 | UNUSED;    // eTPU[20] 
        SIU.PCR[135].R = B0001 | UNUSED; 	// eTPU[21] 
        SIU.PCR[136].R = GPIO  | UNUSED; 	// eTPU[22] 
        SIU.PCR[137].R = B0001 | OUTPUT; 	// eTPU[23] fake cam signal
        SIU.PCR[138].R = B0001 | UNUSED; 	// eTPU[24] 
        SIU.PCR[139].R = B0001 | UNUSED; 	// eTPU[25]  
        SIU.PCR[140].R = B0001 | UNUSED; 	// eTPU[26] pin available, eTPU26 used for MAP window internal
        SIU.PCR[141].R = B0001 | UNUSED; 	// eTPU[27] Stepper PWM signal  Fuel Pump & etpu channel
        SIU.PCR[142].R = B0001 | UNUSED; 	// eTPU[28] pin available, & Knock window internal
        SIU.PCR[143].R = B0001 | UNUSED; 	// eTPU[29] PWM_Pad &  
        SIU.PCR[144].R = B0011 | OUTPUT;    // eTPU[30] toothgen simulator, eTPU30 hardwired to eTPU1 
        SIU.PCR[145].R = B0011 | OUTPUT;    // eTPU[31] toothgen simulator, eTPU31 hardwired to eTPU0
        

        //EMIOS primary
        SIU.PCR[179].R = B0001 | OUTPUT; 	// eMIOS 0 DBW1
        SIU.PCR[180].R = GPIO | UNUSED; 	// eMIOS 1 no pin on the 144 processor
        SIU.PCR[181].R = B0001 | OUTPUT; 	// eMIOS 2 DBW2
        SIU.PCR[183].R = B0001 | OUTPUT; 	// eMIOS 4 DBW3

        SIU.PCR[187].R = B0001 | OUTPUT;    // eMIOS 8
          /* Pins define elsewhere in FS code
        SIU.PCR[188].R = GPIO | OUTPUT;     // eMIOS 9 Pin LED_0 emios 9 used for msec clock
        SIU.PCR[189].R = GPIO | OUTPUT;     // eMIOS 10 Pin LED_1 and emios 10 is reserved for eQADC trigger
        SIU.PCR[190].R = GPIO | OUTPUT;     // eMIOS 11 Pin LED_2 and emios 11 is reserved for eQADC trigger
        SIU.PCR[191].R = GPIO | OUTPUT;     // eMIOS 12 Pin LED_3 and emios 12 is reserved for eQADC trigger
          */
           									// eMIOS 13 no pin on 5634 and emios 13 is reserved for eQADC trigger
        SIU.PCR[193].R = GPIO | OUTPUT;     // eMIOS 14 Pin ,  and emios 14 is reserved for eQADC trigger
        SIU.PCR[194].R = GPIO | OUTPUT; 	// eMIOS no pin on 5634 and emios 15 is reserved for eQADC trigger

        SIU.PCR[202].R = GPIO | UNUSED;;     // eMIOS 23 DBW4
         
        //AN channel functions (do not require PCR settings)
        //AN0 - Knock1 +
        //AN1 - Knock1 -
        //AN2 - 
        //AN3 - 
        //AN9 - MAP_1
        //AN11 -V_Batt
         SIU.PCR[215].R = ALT2 | OUTPUT; 	//AN12 eTPU 
         SIU.PCR[216].R = ALT2 | OUTPUT; 	//AN13    
         SIU.PCR[217].R = ALT2 | OUTPUT; 	//AN14   
         SIU.PCR[218].R = ALT2 | OUTPUT; 	//AN15                
        //AN16 - 
        //AN17 - POT on dev board
        //AN18 - 
        //AN21 - V_P1
        //AN22 - V_MAP_2
        //AN23 - V_O2_1
        //AN24 - V_O2_2 
        //AN25 - V_Batt
        //AN27 - MAP_1
        //AN28 -  
        //AN30 - 
        //AN31 - V_TPS
        //AN32 - V_P3
        //AN33 - V_P4
        //AN34 - V_P5
        //AN35 - V_MAF
        //AN36 - no pin on 144 verison
        //AN37 - no pin on 144 verison
        //AN38 - V_IAT
        //AN39 - V_CLT
        
        
/*
Fuel    eTPU6 eTPU7 eTPU8 eTPU9 eTPU10 eTPU11 eTPU12 eTPU13
Spark   eTPU2 eTPU3 eTPU4 eTPU5
Crank   eTPU0
Cam     eTPU1
*/





/*****************************************************************************************/


/*****************************************************************************************/

#ifdef PWM1_PAD
   // Configure PWM output pad/pin
   SIU.PCR[PWM1_PAD].R = B0001 | OUTPUT;
#endif

}