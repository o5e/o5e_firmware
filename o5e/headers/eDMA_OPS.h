/**************************************************************************/
/* FILE NAME: eDMA_OPS.h                                                  */
/*                                                                        */
/* DESCRIPTION:                                                           */
/* This file contains prototypes and definitions for the MPC5554          */
/* initialize and operate the eDMA.                                       */
/*                                                                        */
/*========================================================================*/
/* REV      AUTHOR          DATE          DESCRIPTION OF CHANGE           */
/* ---      -----------     -----------   ---------------------           */
/* 1.1      J. Zeeff        13/Oct/10     Added serial routines           */
/* 1.0      P. Schlein      27/Jul/10    Initial version.                 */
/**************************************************************************/

void init_eDMA(void);
void Zero_DMA_Channel(int DMA_chan);
