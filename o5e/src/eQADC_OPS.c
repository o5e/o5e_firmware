/*********************************************************************************

        @file   eQADC_OPS.c  
        @author Paul Schlein, Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU  - A/D
        @note   This file contains functions for the MPC5xxx to initialize and run 
        @note   continuous A/D via DMA or DMA triggered by the eTPU
        @version .9
        @copyright 2011, 2012 P. Schlein and Jon Zeeff

*************************************************************************************/

/* 

Portions Copyright 2011 P. Schlein - MIT License
Portions Copyright 2011, 2012  Jon Zeeff - All rights reserved

*/

#include <stdint.h>
#include "mpc563xm.h"
#include "eQADC_OPS.h"

uint32_t ADC_CmdQ0[40];
uint32_t ADC_CmdQ1[1];
uint32_t ADC_CmdQ2[1];
uint32_t ADC_CmdQ3[1];
uint32_t ADC_CmdQ4[1];
uint32_t ADC_CmdQ5[1];

vuint16_t ADC_RsltQ0[40];
vuint16_t ADC_RsltQ1[1];
vuint16_t ADC_RsltQ2[1];
vuint16_t ADC_RsltQ3[1];
vuint16_t ADC_RsltQ4[1];
vuint16_t ADC_RsltQ5[1];

/*******************************************************************************************
 FUNCTION     : init_ADC                                                                   
                                                                                           
 PURPOSE      :  Initilaize the eQADC in 2 Steps-1st-Prep ADCs, 2nd-Config for User        
 INPUT NOTES  :  ADC_CLK Speed                                                             
   12KRPM=200RPS=1200 Intake Events/sec (12 cylinders)                                     
   10 bit accuracy for ADC_CLK<=16mhz
   @132mhz, divide by 22=ob01010, yields 375K Samples/sec MPC5554
                                                                                           
*******************************************************************************************/

void init_ADC(void)
{
    int i;

    // Note:  eQADC Conversion Command Format
    // Controls which A/D converter used, queue #, A/D chan #, timestamps, etc.
    // Note: check Q sizes in the .h file - they must be exactly correct and match below

#define ADC(n) 		(n << (31-6))    // which converter
#define LST(n)          (n << (31-13))	 // extra sample clocks (may reduce noise)
#define RFIFO(n) 	(n << (31-11))   // where to put results
#define CHANNEL(n) 	(n << (31-23))
#define PAUSE 		(1 << (31-1))
#define EOQ 		(1UL << (31-0))

    // Convert all 40 A/D channels (CmdQ 0 -> ResultQ 0), triggered by eMIOS10
    for (i = 0; i < 39;  ++i) 
        ADC_CmdQ0[i] = (uint32_t)(ADC(0) | LST(1) | RFIFO(0) | CHANNEL(i));
    ADC_CmdQ0[39] = ADC(0) | RFIFO(0) | CHANNEL(39) | PAUSE;

    // ADC1, Q1 is triggered by eMIOS11
    ADC_CmdQ1[0] = ADC(1) | RFIFO(1) | CHANNEL(17) | PAUSE;     // Convert POT, AN17
    ADC_CmdQ2[0] = ADC(1) | RFIFO(2) | CHANNEL(40) | PAUSE;     // Convert VRH
    ADC_CmdQ3[0] = ADC(1) | RFIFO(3) | CHANNEL(41) | PAUSE;     // Convert VRL
    ADC_CmdQ4[0] = ADC(1) | RFIFO(4) | CHANNEL(17) | PAUSE;     // Convert POT

    // Q5 is triggered by the eTPU knock window for MAP
    // IMO, replace this with "min MAP seen over last 720 degrees"
    ADC_CmdQ5[0] = ADC(1) | LST(1) | RFIFO(5) | CHANNEL(17) | PAUSE;     // Convert POT, AN17.  TODO change

    // Angle triggered Queue-On ADC2=B/N=1, RFIFO2=Msg Tag=1 , eMIOS 14 trigger
    //ADC_CmdQ3[0] = (3 << 20) | (96 << 8);             // Disable Time Stamp ; 
    //ADC_CmdQ3[1] = (3 << 20) | (97 << 8) | EOQ;       // Disable Time Stamp ; 

    // eMIOS 12 trigger
    //ADC_CmdQ5[0] = (5 << 20) | (17 << 8) | EOQ ;      // Disable Time Stamp ; 

    // Prepare the two ADCs for User Mode                                          

    // eQADC Module Configuration Register
    EQADC.MCR.R = 0x00000000;   // Synchronous Serial Interface-Disable; Debug-Disable

    // eQADC Null Message Send Format Register
    EQADC.NMSFR.R = 0x00000000;

    // eQADC Ext. Trigger Digital Filter Register
    EQADC.ETDFR.R = 0x00000000;

    // WARNING: MPC5634 RM indicates CFCR registers are 32 bit but they are 16 bit

    // eQADC CFIFO Control Registers 
    for (i = 0; i < 6; ++i) {
        EQADC.CFCR[0].R = 0x0000;       // disable
        EQADC.IDCR[0].R = 0x0000;       // no interrupt or DMA
        EQADC.FISR[0].B.EOQF = 1;       // clear flag
    }

    // === start CFIFO0 only code

    // Write Configuration Commands to Internal ADC Registers by using CFIFO

    // PUSH Four Commands Only into CFIFO Using eQADC CFIFO Push Register                        
    // Note:  eQADC Write Configuration Format
    //     First Byte=EOQ, PAUSE Bit, "000", ExtBufrBit,BN bit=ADC0 or 1, R/W=1/0,               
    //     Next 2 Bytes=ADC Register High and Low Bytes,                                         
    //     Last Byte=ADC Register Address-see Table 19-25, 19-40, pg 770                         

#define ADC_ADDRESS(n)  	((n) << 0)
#define ADC_REGISTER(n) 	((n) << 8)
#define BN(n)  		        ((n) << (31-6)) /* which CBUFFER */
#define AD_ENABLE 		(1 << 15)
#define AD_CLOCK(n) 	        ((n) << 0)      /* ADC0/1_CLK_PS[0:4]  - divisor select */

    // eQADC ADC Time Stamp Control Register (0) - not used
    // EQADC.CFPR[0].R = ADC_ADDRESS(2) | 0x0800;   // ADC_TSCR = 0x0008=Clock divide=16     

    // eQADC ADC Time Base Counter Register (0) - not used
    // EQADC.CFPR[0].R = ADC_ADDRESS(3) | 0;        // ADC_TBCR = 0x0000     

    // eQADC Initialize ADC0 Control Register, set A/D clock
    EQADC.CFPR[0].R = ADC_ADDRESS(1) | BN(0) | ADC_REGISTER(AD_ENABLE | AD_CLOCK(0xA));
    // eQADC Initialize ADC1 Control Register, set A/D clock
    EQADC.CFPR[0].R = ADC_ADDRESS(1) | BN(1) | ADC_REGISTER(AD_ENABLE | AD_CLOCK(0xA)) | EOQ;

    // CFIFO Trigger Mode Constants
#define DISABLE_Q 0x0
#define SW_TRIG_SS 0x1
#define LOW_GATED_EXT_SS 0x2
#define HIGH_GATED_EXT_SS 0x3
#define FALLING_EXT_SS 0x4
#define RISING_EXT_SS 0x5
#define FALL_OR_RISE_EXT_SS 0x6
#define SW_TRIG_CS 0x9
#define LOW_GATED_EXT_CS 0xA
#define HIGH_GATED_EXT_CS 0xB
#define FALLING_EXT_CS 0xC
#define RISING_EXT_CS 0xD
#define FALL_OR_RISE_EXT_CS 0xE

    // Trigger CFIFO0 with Software Trigger, single Scan to Configure ADCx's
    EQADC.CFCR[0].B.MODE = SW_TRIG_SS;
    EQADC.CFCR[0].B.SSE = 1;

    // Wait for Queue Done
    while (EQADC.FISR[0].B.EOQF == 0) {
    }                           /* Wait for End Of Queue flag */

    // A/D converters are now initilized

    // ======== above is only queue 0, below is all queues

    // Clear this FISR flag
    for (i = 0; i < 6; ++i)
        EQADC.FISR[i].B.EOQF = 1;

    // Disable Queue in Preparation for Queue Mode Change                                   
    for (i = 0; i < 6; ++i)
        EQADC.CFCR[i].R = DISABLE_Q;    // MODE = 0

    // Wait for Queues IDLE Before Changing to User Mode
    while (EQADC.CFSR.B.CFS0 == 0x3) {
    }
    while (EQADC.CFSR.B.CFS5 == 0x3) {
    }

    // Invalidate Command Queue                                                                  
    for (i = 0; i < 6; ++i)
        EQADC.CFCR[i].B.CFINV = 1;

    // 6 eQADC Interrupt and DMA Control Registers 
    // TODO Change to use bit field #defines
    for (i = 0; i < 6; ++i)
        EQADC.IDCR[i].R = 0x0303;       // DMA for RFIFO and CFIFO
    // CFIFO Underflow Interrupt = 0
    // CFIFO Fill Enable = 1
    // CFIFO Fill Select = 1, via eDMA
    // RFIFO Overflow Interrupt = 0
    // RFIFO Drain = 1
    // RFIFO Drain Select = 1, via eDMA

    // Trigger CFIFOx with rising edge, single scan 
    // doesn't work
    EQADC.CFCR[0].B.MODE = RISING_EXT_SS;       // Rising Edge Ext.Trigger, Single Scan
    EQADC.CFCR[1].B.MODE = RISING_EXT_SS;       // Rising Edge Ext.Trigger, Single Scan
    EQADC.CFCR[2].B.MODE = RISING_EXT_SS;       // Rising Edge Ext.Trigger, Single Scan
    EQADC.CFCR[3].B.MODE = RISING_EXT_SS;       // Rising Edge Ext.Trigger, Single Scan
    EQADC.CFCR[4].B.MODE = RISING_EXT_SS;       // Rising Edge Ext.Trigger, Single Scan
    EQADC.CFCR[5].B.MODE = RISING_EXT_SS;       // Rising Edge Ext.Trigger, Single Scan

    // Set single scan enable to start
    EQADC.CFCR[0].B.SSE = 1;    // Single Scan Enable = 1 
    EQADC.CFCR[1].B.SSE = 1;    // Single Scan Enable = 1
    EQADC.CFCR[0].B.SSE = 1;    // Single Scan Enable = 1 
    EQADC.CFCR[1].B.SSE = 1;    // Single Scan Enable = 1
    EQADC.CFCR[2].B.SSE = 1;    // Single Scan Enable = 1
    EQADC.CFCR[3].B.SSE = 1;    // Single Scan Enable = 1
    EQADC.CFCR[4].B.SSE = 1;    // Single Scan Enable = 1
    EQADC.CFCR[5].B.SSE = 1;    // Single Scan Enable = 1

    // Configure eQADC Trigger Select Registers                          */
    // Triggering of eQADC CFIFOs and 1
    // 00 SIU_ISEL 
    // 01 eTPU_A channel
    // 10 eMIOS channel
    // 11 ETRIG external

    SIU.ETISR.B.TSEL0 = 0x2;    // 0x2 for eMIOS 10
    SIU.ETISR.B.TSEL1 = 0x2;    // eTPU 31 or eMIOS 11
    SIU.ETISR.B.TSEL2 = 0x2;    // eTPU 29 or eMIOS 15
    SIU.ETISR.B.TSEL3 = 0x1;    // eTPU 28 or eMIOS 14  Knock 
    SIU.ETISR.B.TSEL4 = 0x2;    // eTPU 27 or eMIOS 13 
    SIU.ETISR.B.TSEL5 = 0x1;    // eTPU 26 or eMIOS 12  MAP

}                               // init_ADC()
