/**********************************************************************************/
/* FILE NAME: eSCI_OPS.c                                                          */
/*                                                                                */
/* DESCRIPTION:                                                                   */
/* This file contains functions for Initializing the 5554's eSCI Engine           */
/*                                                                                */
/*================================================================================*/
/* ORIGINAL AUTHOR:  Paul Schlein                                                 */
/* REV      AUTHOR          DATE          DESCRIPTION OF CHANGE                    */
/* ---     -----------     ----------    ---------------------                    */
/* 1.0     P. Schlein      22/Sep/11     Initial version                          */
/**********************************************************************************/

#include "config.h"
#include "system.h"

#ifndef MPC5602

#   include "eSCI_OPS.h"

/**********************************************************************************/
/* FUNCTION     : Initialize the eSCI for Queued Operations                       */
/* PURPOSE      :                                                                 */
/* INPUT NOTES  : None                                                            */
/* RETURN NOTES : None                                                            */
/* WARNING      :                                                                 */
/**********************************************************************************/

void init_eSCI(void)
{

/*  Configure eSCI A                                                              */
/*  Configure per PLX Devices Interface- 19200, 8 Data Bits, 1 Stop Bit, No parity*/

/* Configure SCI (serial port) pads-Primary Function, eSCI_A and B, TxDA and RxDA-see RM 2.3.6, 2-40, pg102 */
#define B0001  (1 << 10)

    SIU.PCR[89].R = B0001;
    SIU.PCR[90].R = B0001;
    SIU.PCR[91].R = B0001;
    SIU.PCR[92].R = B0001;

    ESCI_A.CR2.B.BSTP = 0;      //Suppress DMA TX with errors present
    ESCI_A.CR2.B.MDIS = 0;      //Don't disable
    ESCI_A.CR2.B.RXDMA = 1;     //Activate RX DMA Channel
    ESCI_A.CR2.B.TXDMA = 1;     //Activate TX DMA Channel
    ESCI_A.CR2.B.SBSTP = 0;     //Don't stop on bit error

    ESCI_A.CR1.B.SBR = CPU_CLOCK / (16 * BAUD_RATE);    //SCI Baud Rate
    ESCI_A.CR1.B.M = 0;         //1 start bit, 8 bits, 1 stop bit
    ESCI_A.CR1.B.PE = 0;        //Parity Disable
    ESCI_A.CR1.B.PT = 0;        //Parity Type
    ESCI_A.CR1.B.TE = 1;        //Transmitter Enable
    ESCI_A.CR1.B.RE = 1;        //Receiver Enable

    ESCI_A.SR.R = 0xffffffff;   /* Clear flags */

/*  Configure eSCI B                                                              */

    ESCI_B.CR2.B.BSTP = 0;      //Suppress DMA TX with errors present
    ESCI_B.CR2.B.MDIS = 0;      //Don't disable
    ESCI_B.CR2.B.RXDMA = 0;     //Activate RX DMA Channel
    ESCI_B.CR2.B.TXDMA = 0;     //Activate TX DMA Channel
    ESCI_B.CR2.B.SBSTP = 0;     //Don't stop on bit error

    ESCI_B.CR1.B.SBR = CPU_CLOCK / (16 * BAUD_RATE);    //SCI Baud Rate
    ESCI_B.CR1.B.M = 0;         //1 start bit, 8 bits, 1 stop bit
    ESCI_B.CR1.B.PE = 0;        //Parity Disable
    ESCI_B.CR1.B.PT = 0;        //Parity Type
    ESCI_B.CR1.B.TE = 1;        //Transmitter Enable
    ESCI_B.CR1.B.RE = 1;        //Receiver Enable

    ESCI_B.SR.R = 0xffffffff;   /* Clear flags */

}

#endif
