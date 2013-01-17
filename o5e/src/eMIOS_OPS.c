//*****************************************************************************************/
// FILE NAME: eMIOS_OPS.c                                                                 */ 
// DESCRIPTION:                                                                           */ 
// This file contains functions for the MPC5xxx to Initialize the eMIOS Engine            */ 
// 24 Unified Channels Available-channels 10-15 for eQADC Triggering                      */ 
//*****************************************************************************************/ 

// Portions Copyright 2011 P. Schlein - MIT License
// Portions Copyright 2011, 2012  Jon Zeeff - All rights reserved

#include "mpc563xm.h"
#include "config.h"     /**< pickup MSEC_EMIOS_CHANNEL only */
#include "eMIOS_OPS.h"
    
//*****************************************************************************************/ 
// FUNCTION     :  init_eMIOS                                                             */ 
// PURPOSE      :   Initialize 2 eMIOS Unified Channels for eQADC Triggering              */ 
//                  Note:  Ch(10) 100 samples/sec and Ch(11) at 1/sec                     */ 
//                  Also, Initializes CH[09] as a Timer                                   */ 
// INPUT NOTES  :                                                                         */ 
// RETURN NOTES :                                                                         */ 
// WARNING      : For testing, set output pads in SIU_OPS.  Nomal Ops use SIU_ETISR       */ 
// WARNING      : Differnt channes support different modes - read the RM
//*****************************************************************************************/ 

void init_eMIOS(void)
{
    
//  First, configure the eMIOS for xxx mhz/250 internal clock with Module Config Reg        
//  see 5554 RM 17.3.1.1, 17-8, pg 628                                                          

EMIOS.MCR.B.GPRE = 249;    // Prescalar Divisor-NOTE yields 250 divisor              
EMIOS.MCR.B.ETB = 0;       // Disble external timebase, may use Chl 23 with STAC     
EMIOS.MCR.B.GPREN = 1;     // Enable eMIOS clock                                     
EMIOS.MCR.B.GTBE = 1;      // Enable global timebase                                 
EMIOS.MCR.B.FRZ = 0;       // Disable stopping channels in debug                     
EMIOS.OUDR.R = 0;          // 0 = enable outputs
EMIOS.UCDIS.R = 0;         // 0 = enable

//  Second, configure Ch(10) to FAST trigger eQADC CFIFO Ch(0) with OPWFM                  
//  set for xx triggers/second, 10% duty cycle                                            
EMIOS.CH[10].CCR.B.MODE = 0x01;    // Set GPIO Mode to Reset CNTR                             
EMIOS.CH[10].CCR.B.FEN = 0x00;     // Disable Interrupt Request                              
EMIOS.CH[10].CCR.B.UCPRE = 0x3;    // Further Divide by 4 with internal channel prescaler    
EMIOS.CH[10].CCR.B.UCPREN = 1;     // Enable internal channel prescaler                      
EMIOS.CH[10].CADR.R = 2;           // Leading edge at 2 ticks
EMIOS.CH[10].CBDR.R = 4;           // Trailing edge at 4 ticks         
EMIOS.CH[10].CCR.B.BSL = 0x03;     // Use internal counter                                   
EMIOS.CH[10].CCR.B.EDSEL = 1;      // 
EMIOS.CH[10].CCR.B.MODE = 0x58;    // OPWFM mode
  
 
//  Configure Ch(11) to trigger eQADC CFIFO Ch(1) with OPWFM                   
//  set for x trigger/second, 10% duty cycle                                               
EMIOS.CH[11].CCR.B.MODE = 0x01;    // Set GPIO Mode to Reset CNTR                             
EMIOS.CH[11].CCR.B.FEN = 0x00;     // Disable Interrupt Request                              
EMIOS.CH[11].CCR.B.UCPRE = 0x3;    // Further prescaling with internal channel prescaler     
EMIOS.CH[11].CCR.B.UCPREN = 1;     // Enable internal channel prescaler                      
EMIOS.CH[11].CADR.R = 2;     	   // Leading edge    
EMIOS.CH[11].CBDR.R = 4;         // Trailing edge        
EMIOS.CH[11].CCR.B.BSL = 0x03;     // Use internal counter                                   
EMIOS.CH[11].CCR.B.EDSEL = 1;      // 
EMIOS.CH[11].CCR.B.MODE = 0x58;    // OPWFM mode

// turn these on for debug
int i;
for (i = 14; i <= 15; ++i) {
// For A/D trigger and output on a pin
EMIOS.CH[i].CCR.B.MODE = 0x01;    // Set GPIO Mode to Reset CNTR                             
EMIOS.CH[i].CCR.B.FEN = 0x00;     // Disable Interrupt Request                              
EMIOS.CH[i].CCR.B.UCPRE = 0x3;    // Further prescaling with internal channel prescaler     
EMIOS.CH[i].CCR.B.UCPREN = 1;     // Enable internal channel prescaler                      
EMIOS.CH[i].CADR.R = 2;           // Leading edge    
EMIOS.CH[i].CBDR.R = 4;           // Trailing edge        
EMIOS.CH[i].CCR.B.BSL = 0x03;     // Use internal counter                                   
EMIOS.CH[i].CCR.B.EDSEL = 1;      // 
EMIOS.CH[i].CCR.B.MODE = 0x58;    // OPWFM mode 
} // for
    
//  Configure a free running timer for general purpose and OS use               
//  Note:  with Prescalars set to 250 (249+1) and 4 (3+1), this counter runs at system clock speed / 1000

EMIOS.CH[MSEC_EMIOS_CHANNEL].CCR.B.FEN = 0x00;     // Disable Interrupt Request                               
EMIOS.CH[MSEC_EMIOS_CHANNEL].CCR.B.UCPRE = 0x3;    // Further 1/4x with internal channel prescaler      
EMIOS.CH[MSEC_EMIOS_CHANNEL].CCR.B.UCPREN = 1;     // Enable internal channel prescaler                       
EMIOS.CH[MSEC_EMIOS_CHANNEL].CADR.R = 0xffffff;    // run free - no modulus
EMIOS.CH[MSEC_EMIOS_CHANNEL].CCR.B.BSL = 0x03;     // Use internal counter                                    
EMIOS.CH[MSEC_EMIOS_CHANNEL].CCR.B.EDSEL = 1;      // 
EMIOS.CH[MSEC_EMIOS_CHANNEL].CCR.B.MODE = 0x50; // Set MC up buffer mode                                   
// enable STM counter in case we want to use it for measuring code execution speed
STM.CR.R = 0x00003;     // enable, no divisor, stop with debug
} // init_eMIOS()
