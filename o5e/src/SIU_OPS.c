/******************************************************************************************/
/* FILE NAME: SIU_OPS.c                                                                   */
/*                                                                                        */
/* DESCRIPTION:                                                                           */
/* This file contains functions for the MPC5xxx to initialize the SIU                     */
/* Ie, what function each external pin is used for                                        */
/*                                                                                        */
/*========================================================================================*/
/* REV      AUTHOR         DATE         DESCRIPTION OF CHANGE                             */
/* ---      -----------    ----------   ---------------------                             */
/* 5.1      M.Eberhardt    5/Jan/12     Added all pins and user config for 5554, 5634     */
/* 5.0      J. Zeeff       14/Nov/11    Made bit functions generic                        */
/* 4.0      P. Schlein     20/Sep/11    Add Two eSCI Input/Output                         */
/* 3.0      P. Schlein     4/Sep/11     Add Fuel Pump Output                              */
/* 2.0      P. Schlein     28/July/11   Add eMIOS to eQADC Triggering                     */
/* 2.0      P. Schlein     20/June/11   Add 6 SPARK and FUEL outputs                      */
/* 1.0      P. Schlein     11/June/11   Initial version                                   */
/******************************************************************************************/

#include "config.h"
#include "system.h"
#include "cpu.h"
#include "SIU_OPS.h"
#include "variables.h"

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

#ifdef PHYCORE
    //Configure  for CRANK (TCRCLKA)
    SIU.PCR[113].R = PRIMARY | INPUT;

    // eTPUa primary
    SIU.PCR[114].R = GPIO | OUTPUT;     // P13
    SIU.PCR[115].R = PRIMARY | INPUT;   // cam sensor input
    SIU.PCR[116].R = PRIMARY | OUTPUT;  //fixed LED
    SIU.PCR[117].R = GPIO | OUTPUT;     // P14
    SIU.PCR[118].R = Config_HS_1 | UNUSED;      //HS1, no driver
    SIU.PCR[119].R = GPIO | OUTPUT;     // P12
    SIU.PCR[120].R = GPIO | UNUSED;     //unused
    SIU.PCR[121].R = GPIO | OUTPUT;     // P10
    SIU.PCR[122].R = CONFIG_IGN | OUTPUT;     // ignition 1
    SIU.PCR[123].R = CONFIG_IGN | OUTPUT;     // ignition 2
    SIU.PCR[124].R = CONFIG_IGN | OUTPUT;     // ignition 3
    SIU.PCR[125].R = CONFIG_IGN | OUTPUT;     // ignition 4
    SIU.PCR[126].R = CONFIG_IGN | OUTPUT;     // ignition 5
    SIU.PCR[127].R = CONFIG_IGN | OUTPUT;     // ignition 6
    SIU.PCR[128].R = GPIO | OUTPUT;     // P11
    SIU.PCR[129].R = GPIO | OUTPUT;     // P08
    SIU.PCR[130].R = GPIO | OUTPUT;     // P09
    SIU.PCR[131].R = GPIO | OUTPUT;     // P06
    SIU.PCR[132].R = Config_LS_3 | UNUSED;      // low side 3, redirected to inj20 on gen1 board
    SIU.PCR[133].R = GPIO | OUTPUT;     // P04
    SIU.PCR[134].R = GPIO | OUTPUT;     // P07
    SIU.PCR[135].R = GPIO | OUTPUT;     // P02
    SIU.PCR[136].R = GPIO | OUTPUT;     // P05
    SIU.PCR[137].R = GPIO | UNUSED;     // 
    SIU.PCR[138].R = GPIO | OUTPUT;     // P03
    SIU.PCR[139].R = GPIO | OUTPUT;     // Power Enable
    SIU.PCR[140].R = GPIO | UNUSED;     // unused
    SIU.PCR[141].R = GPIO | UNUSED;     // unused
    SIU.PCR[142].R = PRIMARY | OUTPUT;  // output for TOOTHGEN, eTPUA 28
    SIU.PCR[143].R = PRIMARY | OUTPUT;  // output for TOOTHGEN, eTPUA 29
    SIU.PCR[144].R = GPIO | UNUSED;     // unused
    SIU.PCR[145].R = GPIO | UNUSED;     // unused

    // engine B primary
    SIU.PCR[147].R = GPIO | OUTPUT;     // P01
    SIU.PCR[148].R = GPIO | UNUSED;     // unused
    SIU.PCR[149].R = GPIO | OUTPUT;     // CJ125 2 DIAHG
    SIU.PCR[150].R = GPIO | UNUSED;     // unused
    SIU.PCR[151].R = GPIO | OUTPUT;     // CJ125 1 DIAHG
    SIU.PCR[152].R = GPIO | UNUSED;     // HS3, no driver
    SIU.PCR[153].R = GPIO | OUTPUT;     // stepper steps
    SIU.PCR[154].R = GPIO | UNUSED;     // unused
    SIU.PCR[155].R = CONFIG_INJ | OUTPUT;     // injector 1
    SIU.PCR[156].R = CONFIG_INJ | OUTPUT;     // injector 2
    SIU.PCR[157].R = CONFIG_INJ | OUTPUT;     // injector 3
    SIU.PCR[158].R = CONFIG_INJ | OUTPUT;     // injector 4
    SIU.PCR[159].R = CONFIG_INJ | OUTPUT;     // injector 5
    SIU.PCR[160].R = CONFIG_INJ | OUTPUT;     // injector 6
    SIU.PCR[161].R = CONFIG_INJ | OUTPUT;     // injector 7
    SIU.PCR[162].R = CONFIG_INJ | OUTPUT;     // injector 8
    SIU.PCR[163].R = CONFIG_INJ | OUTPUT;     // injector 9
    SIU.PCR[164].R = CONFIG_INJ | OUTPUT;    // injector 10
    SIU.PCR[165].R = CONFIG_INJ | OUTPUT;    // injector 11
    SIU.PCR[166].R = CONFIG_INJ | OUTPUT;    // injector 12
    SIU.PCR[167].R = CONFIG_INJ | OUTPUT;    // injector 13
    SIU.PCR[168].R = CONFIG_INJ | OUTPUT;    // injector 14
    SIU.PCR[169].R = CONFIG_INJ | OUTPUT;    // injector 15
    SIU.PCR[170].R = CONFIG_INJ | OUTPUT;    // injector 16
    // TODO - Wheel speeds
    SIU.PCR[171].R = PRIMARY | INPUT;
    SIU.PCR[172].R = PRIMARY | INPUT;
    SIU.PCR[173].R = PRIMARY | INPUT;
    SIU.PCR[174].R = PRIMARY | INPUT;
    SIU.PCR[175].R = GPIO | UNUSED;     // HS2, no driver
    SIU.PCR[176].R = GPIO | OUTPUT;     // stepper config bit MS1
    SIU.PCR[177].R = GPIO | OUTPUT;     // stepper config bit MS1
    SIU.PCR[178].R = GPIO | OUTPUT;     // stepper config bit MS1

    //EMIOS primary
    SIU.PCR[179].R = GPIO | OUTPUT;     	// Flashing LED 
    SIU.PCR[180].R = Config_LS_4 | UNUSED;      // low side4, redirected to inj22 on gen1 board
    SIU.PCR[181].R = CONFIG_IGN | OUTPUT;     // ign 7
    SIU.PCR[182].R = CONFIG_IGN | OUTPUT;     // ign 8
    SIU.PCR[183].R = CONFIG_IGN | OUTPUT;     // ign 9
    SIU.PCR[184].R = CONFIG_IGN | OUTPUT;    // ign 10
    SIU.PCR[185].R = CONFIG_IGN | OUTPUT;    // ign 11
    SIU.PCR[186].R = CONFIG_IGN | OUTPUT;    // ign 12
    SIU.PCR[187].R = GPIO | UNUSED;     	// unused
    SIU.PCR[188].R = Config_HS_4 | UNUSED;      // HS4, no driver on gen1 board
    SIU.PCR[189].R = GPIO | UNUSED;     // eMIOS (10) channel reserved for internal use, GPIO only available this pin, unused
    SIU.PCR[190].R = GPIO | UNUSED;     // eMIOS (11) channel reserved for internal use, GPIO only available this pin, unused
    SIU.PCR[191].R = GPIO | UNUSED;     // eMIOS (12) channel reserved for internal use, GPIO only available this pin, unused
    SIU.PCR[192].R = GPIO | UNUSED;     // eMIOS (13) channel reserved for internal use, GPIO only available this pin, unused
    SIU.PCR[193].R = GPIO | UNUSED;     // eMIOS (14) channel reserved for internal use, GPIO only available this pin, unused
    SIU.PCR[194].R = GPIO | UNUSED;     // eMIOS (15) channel reserved for internal use, GPIO only available this pin, unused
    SIU.PCR[195].R = CONFIG_INJ | OUTPUT;    // injector 17
    SIU.PCR[196].R = CONFIG_INJ | OUTPUT;    // injector 18
    SIU.PCR[197].R = B0000 | OUTPUT;     	// inj19, redirected to low side1 on gen1 board for fuel pump
    SIU.PCR[198].R = B0010 | OUTPUT; 		// inj20, redirected to low side2 on gen1 board for tach
    SIU.PCR[199].R = Config_LS_3 | OUTPUT; // inj21, redirected to low side3 on gen1 board
    SIU.PCR[200].R = Config_LS_4 | OUTPUT; // inj22, redirected to low side4 on gen1 board
    SIU.PCR[201].R = Config_LS_5 | OUTPUT; // inj23, redirected to low side5 on gen1 board
    SIU.PCR[202].R = Config_LS_6 | OUTPUT; // inj24, redirected to low side6 on gen1 board
    //GPIO
    SIU.PCR[203].R = Config_LS_1 | UNUSED;      // low side1 redirected to inj19 on gen1 board
    SIU.PCR[204].R = Config_LS_1 | UNUSED;      // low side2 redirected to inj20 on gen1 board
    SIU.PCR[205].R = GPIO | UNUSED;
    SIU.PCR[206].R = GPIO | UNUSED;
    SIU.PCR[207].R = GPIO | UNUSED;	// eMIOS (12) reserved for A/D use
#endif

/*****************************************************************************************/
#ifdef TRK_MPC5634

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
        SIU.PCR[137].R = GPIO | OUTPUT; 	// fake cam pin for semisequentail or eTPU[23]
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
        SIU.PCR[188].R = B0001 | OUTPUT;        // eMIOS 9 (B0001) or injector 10 eTPU[9] (B0010)    
        SIU.PCR[189].R = GPIO | OUTPUT; 	// flashing LED or eMIOS (10)  
        SIU.PCR[190].R = GPIO | OUTPUT; 	// flashing LED or eMIOS (11)  
        SIU.PCR[191].R = B0001 | OUTPUT;        // eMIOS 12 (B0001) or injector 11 eTPU[27](B0100)

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

#endif

#ifdef PWM1_PAD
   // Configure PWM output pad/pin
   SIU.PCR[PWM1_PAD].R = B0001 | OUTPUT;
#endif

}

// reset the CPU
void reset_cpu(void)
{
    SIU.SRCR.B.SSR = 1;  // software system reset
}
