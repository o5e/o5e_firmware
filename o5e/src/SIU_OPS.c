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
        SIU.PCR[115].R = B0001 | INPUT;       	// cam sensor input eTPU1
        SIU.PCR[116].R = B0001 | OUTPUT; 	// eTPU[2] ignition 1
        SIU.PCR[117].R = B0001 | OUTPUT; 	// eTPU[3] ignition 2
        SIU.PCR[118].R = B0001 | OUTPUT; 	// eTPU[4] ignition 3
        SIU.PCR[119].R = B0001 | OUTPUT; 	// eTPU[5] injector 1
        SIU.PCR[120].R = B0001 | OUTPUT; 	// eTPU[6] ignition 5
        SIU.PCR[121].R = B0001 | OUTPUT; 	// eTPU[7] ignition 6
        SIU.PCR[122].R = B0001 | OUTPUT; 	// eTPU[8] ignition 4
        SIU.PCR[123].R = B0001 | OUTPUT; 	// eTPU[9] injector 2
        SIU.PCR[124].R = B0001 | OUTPUT; 	// eTPU[10] injector 3
        SIU.PCR[125].R = B0001 | OUTPUT; 	// eTPU[11] injector 4
        SIU.PCR[126].R = B0001 | OUTPUT; 	// eTPU[12] injector 5
        SIU.PCR[127].R = B0001 | OUTPUT; 	// eTPU[13] injector 6
        SIU.PCR[128].R = GPIO | UNUSED; 	// eTPU[14] pin available but etpu channel reserved forinjector 9
        SIU.PCR[129].R = GPIO | UNUSED; 	// eTPU[15] pin available but etpu channel reserved forinjector 10
        SIU.PCR[130].R = B0001 | OUTPUT;        // eTPU[16] Tach signal
        SIU.PCR[131].R = B0001 | INPUT;         // eTPU[17] wheelspeed FL
        SIU.PCR[132].R = B0001 | INPUT;         // eTPU[18] wheelspeed FR
        SIU.PCR[133].R = B0001 | INPUT;         // eTPU[19] wheelspeed RL
        SIU.PCR[134].R = B0001 | INPUT;         // eTPU[20] wheelspeed RR
        SIU.PCR[135].R = B0001 | OUTPUT; 	// eTPU[21] injector 7
        SIU.PCR[136].R = B0001 | OUTPUT; 	// eTPU[22] injector 8
        SIU.PCR[137].R = GPIO | OUTPUT; 	// eTPU[23] fake cam signal
        SIU.PCR[138].R = GPIO | UNUSED; 	// eTPU[24]
        SIU.PCR[139].R = GPIO | UNUSED; 	// eTPU[25]  
        SIU.PCR[140].R = B0001 | OUTPUT; 	// eTPU[26] MAP window
        SIU.PCR[141].R = GPIO | UNUSED; 	// eTPU[27] pin available but etpu channel reserved for injector 11
        SIU.PCR[142].R = B0001 | OUTPUT; 	// eTPU[28] Knock window
        SIU.PCR[143].R = GPIO | UNUSED; 	// eTPU[29] pin available but etpu channel reserved for injector 12
        SIU.PCR[144].R = B0011 | OUTPUT;      	// eTPU[30] toothgen simulator, eTPU30 hardwired to eTPU1 
        SIU.PCR[145].R = B0011 | OUTPUT;      	// eTPU[31] toothgen simulator, eTPU31 hardwired to eTPU0 

        //EMIOS primary
        SIU.PCR[179].R = GPIO | OUTPUT; 	// eMIOS 0 fuel pump
        SIU.PCR[180].R = GPIO | UNUSED; 	// eMIOS 1
        SIU.PCR[181].R = GPIO | UNUSED; 	// eMIOS 2
        SIU.PCR[183].R = B0001 | OUTPUT; 	// eMIOS 4 (B0001)

        SIU.PCR[187].R = B0001 | OUTPUT;        // eMIOS 8 (B0001) or injector 9 eTPU[8] (B0010) 

        SIU.PCR[192].R = B0001 | OUTPUT; 	// eMIOS 13 (B0001) or GPIO 
        SIU.PCR[193].R = B0001 | OUTPUT;        // eMIOS 14 (B0001) or injector 12 eTPU[29](B0100)
        SIU.PCR[194].R = B0001 | OUTPUT; 	// eMIOS 15 (B0001)

        SIU.PCR[202].R = GPIO | UNUSED;         // eMIOS (23)
         
        //AN channel functions (do not require PCR settings)
        //AN1 - Knock +
        //AN2 - Knock -
        //AN9 - MAP_1
        //AN11 - V_Batt
        //AN16 - V_TPS
        //AN17 - POT on dev board
        //AN18 - V_IAT
        //AN21 - V_CLT
        //AN22 - V_MAP_2
        //AN23 - V_MAP_3
        //AN24 - V_O2_1_UA
        //AN25 - V_O2_1_UR
        //AN27 - V_O2_2_UA
        //AN28 - V_O2_2_UR
        //AN30 - V_P1
        //AN31 - V_P2
        //AN32 - V_P3
        //AN33 - V_P4
        //AN34 - V_P5
        //AN35 - V_P6

/*
Fuel    eTPU5 eTPU11 eTPU12 eTPU13 eTPU14 eTPU15 eTPU21 eTPU22 eTPU8 eTPU9 eTPU27 eTPU29
Spark   eTPU2 eTPU3 eTPU4 eTPU8 eTPU6 eTPU7 
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