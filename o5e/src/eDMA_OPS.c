/*********************************************************************************

        @file   eDMA_OPS.c
        @author P. Schlein and Jon Zeeff 
        @date   May 19, 2012
        @brief  Open5xxxECU  - eDMA
        @note   
        @version  1.0
        @copyright 2011 Jon Zeeff and P. Schlein

***********************************************************************************/

/* 

Portions Copyright 2011 P. Schlein - MIT License
Portions Copyright 2011 Jon Zeeff - All rights reserved

*/
/*========================================================================================*/
/* REV      AUTHOR         DATE          DESCRIPTION OF CHANGE                            */
/* ---      -----------    ----------    ---------------------                            */
/* 5.0      J. Zeeff       09/Jan/12     fixes + restructured to be maintainable          */
/* 1.0      P. Schlein     28/July/10    Initial version                                  */

#include <stdint.h>
#include "mpc563xm.h"
#include "eDMA_OPS.h"
#include "err.h"
#include "eQADC_OPS.h"

static void Init_AD_DMA(int DMA_chan, void *cmd_source, void *cmd_dest, int cmd_count, void *rec_source, void *rec_dest, int rec_count);

/******************************************************************************************/
/* FUNCTION     :  init_eDMA                                                              */
/* PURPOSE      :   Initialize the eDMA engine for eQADC_OPS and eSCI_OPS                 */
/*                    Note:  Major Reference,  'MPC5554 Revealed', pg 194-196             */
/* INPUT NOTES  :                                                                         */
/* RETURN NOTES :                                                                         */
/* WARNING      : eDMA Channel 0, 2, 4, 6, 8, 10 Assigned to eQADC CFIFO 0-5 and          */
/*                eDMA Channel 1, 3, 5, 7, 9, 11 Assigned to eQADC RFIFO 0-5              */
/******************************************************************************************/
void init_eDMA(void)
{
    /*  eQADC Configuration                                                                   */
    /* Define Locations for eQADC POP and PUSH registers-Base Address=FFF8-0000               */
    #define EQADC_BASE 	  0xFFF80000
    #define CFIFO0_PUSH   (EQADC_BASE + 0x10)	// 32 bits
    #define CFIFO1_PUSH   (EQADC_BASE + 0x14)        
    #define CFIFO2_PUSH   (EQADC_BASE + 0x18)        
    #define CFIFO3_PUSH   (EQADC_BASE + 0x1C)   
    #define CFIFO4_PUSH   (EQADC_BASE + 0x20)        
    #define CFIFO5_PUSH   (EQADC_BASE + 0x24)

    #define RFIFO0_POP    (EQADC_BASE + 0x32)
    #define RFIFO1_POP    (EQADC_BASE + 0x36)	 /* Lower 16 Bits */
    #define RFIFO2_POP    (EQADC_BASE + 0x3A)	 
    #define RFIFO3_POP    (EQADC_BASE + 0x3E)	 
    #define RFIFO4_POP    (EQADC_BASE + 0x42)	 
    #define RFIFO5_POP    (EQADC_BASE + 0x46)	 

    /*  eDMA Configuration Register -see RM 9.3.1.1, 9-9, pg 321;  'Revealed', pg 194         */
    EDMA.CR.R = 0x0000E400;     // Group Arbitration is Fixed Priority
    // Channel Arbitration is Fixed Priority
    // Group 3 Priority is 3
    // Group 2 Priority is 2 
    // Group 1 Priority is 1 
    // Group 0 Priority is 0 
    // Debug is disabled  

    /*  eDMA Enable Request Registers-see RM 9.3.1.4, 9-12, pg 324                            */
    EDMA.ERQRL.R = 0x00000000;  // Low channels: Disabled 

    /*  eDMA Enable Error Interrupt Request Registers-see RM 9.3.1.3, 9-14, pg 326            */
    EDMA.EEIRL.R = 0x00000000;  // Low channels-0-31

    // SCI DMA is done later in a different file

    // Initialize A/D DMA channels that are being used (caution - time stamps are not supported)
    // Note: commands are 4 bytes, results are 2 bytes
    Init_AD_DMA(0,  &ADC_CmdQ0, (void *)CFIFO0_PUSH, sizeof(ADC_CmdQ0) / 4, (void *)RFIFO0_POP, &ADC_RsltQ0, sizeof(ADC_RsltQ0) / 2);
    Init_AD_DMA(2,  &ADC_CmdQ1, (void *)CFIFO1_PUSH, sizeof(ADC_CmdQ1) / 4, (void *)RFIFO1_POP, &ADC_RsltQ1, sizeof(ADC_RsltQ1) / 2);
    Init_AD_DMA(4,  &ADC_CmdQ2, (void *)CFIFO2_PUSH, sizeof(ADC_CmdQ2) / 4, (void *)RFIFO2_POP, &ADC_RsltQ2, sizeof(ADC_RsltQ2) / 2);
    Init_AD_DMA(6,  &ADC_CmdQ3, (void *)CFIFO3_PUSH, sizeof(ADC_CmdQ3) / 4, (void *)RFIFO3_POP, &ADC_RsltQ3, sizeof(ADC_RsltQ3) / 2);
    Init_AD_DMA(8,  &ADC_CmdQ4, (void *)CFIFO4_PUSH, sizeof(ADC_CmdQ4) / 4, (void *)RFIFO4_POP, &ADC_RsltQ4, sizeof(ADC_RsltQ4) / 2);
    Init_AD_DMA(10, &ADC_CmdQ5, (void *)CFIFO5_PUSH, sizeof(ADC_CmdQ5) / 4, (void *)RFIFO5_POP, &ADC_RsltQ5, sizeof(ADC_RsltQ5) / 2);

    // Check for DMA errors
    if (EDMA.ESR.R != 0)
       err_push( CODE_OLDJUNK_FF );

} // init_eDMA()


// Set up the DMA controller for A/D

void
Init_AD_DMA(int DMA_chan, void *cmd_source, void *cmd_dest, int cmd_count, void *rec_source, void *rec_dest, int rec_count)
{
    if (cmd_count != rec_count) {
       err_push( CODE_OLDJUNK_FE );       
    }

    // Think of these as memcpy() subroutines that gets executed whenever a given DMA channel is triggered

    EDMA.CERQR.R = (uint8_t)DMA_chan;     		// disable this channel
    EDMA.CERQR.R = (uint8_t)DMA_chan+1;     		// disable this channel

    /* Transfer Control Descriptor for CFIFO 00 - CH0-see RM 9.3.1.16, 9-23, pg 335          */
    EDMA.TCD[DMA_chan].SADDR = (uint32_t)cmd_source;   //Start Address
    EDMA.TCD[DMA_chan].DADDR = (uint32_t)cmd_dest;    //Destination Address
    EDMA.TCD[DMA_chan].DSIZE = 0x02;   //Destination Transfer Size:32 bits
    EDMA.TCD[DMA_chan].SSIZE = 0x02;   //Source Transfer Size:32 bits
    EDMA.TCD[DMA_chan].SOFF = 0x4;     //Signed Source Address Offset in bytes
    EDMA.TCD[DMA_chan].NBYTES = 0x4;   //Inner 'Minor' Byte Count
    EDMA.TCD[DMA_chan].SLAST = cmd_count * -4;    //Signed, # bytes to get Source Address Adjust Back to Start
    EDMA.TCD[DMA_chan].DOFF = 0x0;     //Signed Destination Address Offset
    EDMA.TCD[DMA_chan].DLAST_SGA = 0x0;        //Signed Destination Address Adjust
    EDMA.TCD[DMA_chan].BITER = cmd_count; 	//Beginning 'Major' Iteration Count
    EDMA.TCD[DMA_chan].CITER = cmd_count; 	//Current 'Major' Iteration Count
    EDMA.TCD[DMA_chan].D_REQ = 0x0;    //Disables DMA Channel When Done
    Zero_DMA_Channel(DMA_chan);

    /* Transfer Control Descriptor for RFIFO 00 - CH1-see RM 9.3.1.16, 9-23, pg 335          */
    EDMA.TCD[DMA_chan+1].SADDR = (uint32_t)rec_source;     //Start Address
    EDMA.TCD[DMA_chan+1].DADDR = (uint32_t)rec_dest;       //Destination Address
    EDMA.TCD[DMA_chan+1].DSIZE = 0x01;   //Destination Transfer Size:16 bits
    EDMA.TCD[DMA_chan+1].SSIZE = 0x01;   //Source Transfer Size:16 bits
    EDMA.TCD[DMA_chan+1].SOFF = 0x0;     //Signed Source Address Offset
    EDMA.TCD[DMA_chan+1].NBYTES = 0x2;   //Inner 'Minor' Byte Count
    EDMA.TCD[DMA_chan+1].SLAST = 0x0;    //Signed Source Address Adjust
    EDMA.TCD[DMA_chan+1].DOFF = 0x2;     //Signed Destination Address Offset in bytes
    EDMA.TCD[DMA_chan+1].DLAST_SGA = rec_count * -2; //Signed Dest Address Adjust Back to Start - # bytes
    EDMA.TCD[DMA_chan+1].BITER = rec_count; 	//Beginning 'Major' Iteration Count
    EDMA.TCD[DMA_chan+1].CITER = rec_count; 	//Current 'Major' Iteration Count:  Disabled
    EDMA.TCD[DMA_chan+1].D_REQ = 0x0;    //Disables DMA Channel When Done
    Zero_DMA_Channel(DMA_chan+1);

    //EDMA.CPR[DMA_chan].R   = 0x04;      // Priority x, Channel Preemption is Disabled
    //EDMA.CPR[DMA_chan+1].R = 0x05;      // Priority x, Channel Preemption is Disabled

    EDMA.SERQR.R = (uint8_t)DMA_chan;     	    // enable this channel
    EDMA.SERQR.R = (uint8_t)DMA_chan+1;     	    // enable this channel
}

// The values we don't use

void
Zero_DMA_Channel(int DMA_chan)
{
    EDMA.TCD[DMA_chan].SMOD = 0x00;    //Source Address Modulo
    EDMA.TCD[DMA_chan].DMOD = 0x00;    //Destination Address Modulo
    EDMA.TCD[DMA_chan].BWC = 0x0;      //Bandwidth control:  No DMA Stalls
    EDMA.TCD[DMA_chan].MAJORLINKCH = 0x00;     //Major Channel Number
    EDMA.TCD[DMA_chan].MAJORE_LINK = 0x0;      //Major Channel Link:  Disabled
    EDMA.TCD[DMA_chan].DONE = 0x00;    //Channel Done
    EDMA.TCD[DMA_chan].ACTIVE = 0x00;  //Channel Active
    EDMA.TCD[DMA_chan].E_SG = 0x0;     //Enable Scatter/Gather:  Disabled
    EDMA.TCD[DMA_chan].INT_HALF = 0x0; //Interrupt on Minor Loop Count:  Disabled
    EDMA.TCD[DMA_chan].INT_MAJ = 0x0;  //Interrupt on Major Loop Count Completion
    EDMA.TCD[DMA_chan].START = 0x00;   //Explicit Channel Start Bit
    EDMA.TCD[DMA_chan].BITERE_LINK = 0;        // Linking Not Used
    EDMA.TCD[DMA_chan].CITERE_LINK = 0;        // Linking Not Used
}
