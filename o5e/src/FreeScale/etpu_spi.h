/**************************************************************************
 * FILE NAME: $RCSfile: etpu_spi.h,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the ETPU Serial	    *
 * Peripheral Interface (SPI) API.                                        *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_spi.h,v $
 * Revision 1.2  2004/12/06 14:29:41  r47354
 * Post review comments
 *
 * Revision 1.1  2004/12/02 15:33:36  r47354
 * Initial version.
 *                                                    
 *........................................................................*
 * 0.1   J. Scott     10/May/04  Initial version.                         *
 * 0.2   G. Emerson   7/July/04  GCT modifications                        *
 * 0.3   G. Emerson   16/July/04 Add disclaimer                           *
 * 0.4   G. Emerson   05/Aug/04	 Change timer to timebase                 *
 * 0.5   G. Emerson   09/Sep/04	 Change baud_rate comments                *
 * 0.6   G. Emerson   15/Oct/04     Fix clock_channel desc.               *
 *                               Add clock channel assignment comment blk *
 * 0.7   G. Emerson    1/Nov/04  Remove path in #include                  *
 **************************************************************************/
#include "etpu_spi_auto.h"	/* Auto generated header file for eTPU SPI */


/**************************************************************************/
/*                            Definitions                                 */
/**************************************************************************/

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/* Channel assignment 
Receive = clock_channel - 1
Clock = clock_channel
Transmit = clock_channel + 1
*/

/* New eTPU functions */
/* SPI initialization */

/******************************************************************************
FUNCTION     : fs_etpu_spi_init
PURPOSE      : To initialize three eTPU channels for the SPI function.
INPUTS NOTES : This function has 6 parameters:
          clock_channel - This is the number of the clock channel.
                          1-30 for ETPU_A and 65-94 for ETPU_B.
                          The recieve channel is clock_channel number - 1.
                          The transmit channel is clock_channel number + 1.	  
               priority	- This is the priority to assign to the channel.
               			      This parameter should be assigned a value of:
                          FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                          FS_ETPU_PRIORITY_LOW.
               baud_rate- This is the required rate of data transmission.
                          Maximum baud rate is dependant upon system frequency,
                          timebase frequency and eTPU loading; see eTPU SPI
                          App Note section 4.1.
                          The eTPU microcode schedules an event every ‘half period’ 
                          of the programmed eTPU SPI baud_rate. The half period is 
                          calculated by the initialisation API according to the 
                          following formula
                          
                          half_period = timebase_freq/(2* baud_rate).
                          If this formula resolves to a non-integer value then 
                          the API will effectively round down half_period. This 
                          will result in an error in the baud_rate. The accuracy of
                          data transfers is unaffected by this error.
              shift_dir - Selection of MSB or LSB first shift direction.
               			      This parameter should be assigned a value of
               			      FS_ETPU_SPI_SHIFT_DIR_MSB or FS_ETPU_SPI_SHIFT_DIR_LSB
               polarity	-	This is the polarity of the clock. This parameter
                          should be assigned a value of:
                          FS_ETPU_SPI_CLK_POL_POS or FS_ETPU_SPI_CLK_POL_NEG
	        transfer_size - This is the size of transfer for Transmit and Receive
	       			            This parameter should be assigned a value of between
	       			            0x1 and 0x18 (1 and 24 decimal, 1 and 11000 binary)
               timebase	-	This is the timer to use as a reference for the SPI
                         	clock. This parameter should be assigned to a value
                         	of: FS_ETPU_TCR1 or FS_ETPU_TCR2.
          timebase_freq - This is the prescaled frequency of the timer supplied
                          to the function
******************************************************************************/
int32_t fs_etpu_spi_init( uint8_t clock_channel, uint8_t priority, 
                          uint32_t baud_rate, uint8_t shift_dir,
                          uint8_t polarity, uint32_t transfer_size, 
				                  uint8_t timer, uint32_t timer_freq);
				
/* SPI Transmit Data */

/******************************************************************************
FUNCTION     : fs_etpu_spi_transmit_data
PURPOSE      : For the eTPU to both Transmit and Receive Data.
INPUTS NOTES : This function has 2 parameters:
               clock_channel - This is the number of the clock channel.
                           	  0-31 for ETPU_A and 64-95 for ETPU_B.
               tx_data - This is the data to be transmitted.
               			     It will be placed in data_reg to be read by the
               			     SPI function
RETURNS NOTES: none
******************************************************************************/
void fs_etpu_spi_transmit_data( uint8_t clock_channel,uint32_t tx_data);

/* SPI get data register */

/******************************************************************************
FUNCTION     : fs_etpu_spi_get_data
PURPOSE      : To retrieve the data_reg data from the eTPU SPI channel.
INPUTS NOTES : This function has 1 parameter:
               clock_channel - This is the number of the clock channel.
                           	  0-31 for ETPU_A and 64-95 for ETPU_B.

RETURNS NOTES: none
******************************************************************************/
uint32_t fs_etpu_spi_get_data( uint8_t clock_channel);

/*********************************************************************
 *
 * Copyright:
 *	Freescale Semiconductor, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is
 *  retained without alteration in any modified and/or redistributed
 *  versions, and that such modified versions are clearly identified
 *  as such. No licenses are granted by implication, estoppel or
 *  otherwise under any patents or trademarks of Freescale
 *  Semiconductor, Inc. This software is provided on an "AS IS"
 *  basis and without warranty.
 *
 *  To the maximum extent permitted by applicable law, Freescale
 *  Semiconductor DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,
 *  INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *  PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
 *  REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)
 *  AND ANY ACCOMPANYING WRITTEN MATERIALS.
 *
 *  To the maximum extent permitted by applicable law, IN NO EVENT
 *  SHALL Freescale Semiconductor BE LIABLE FOR ANY DAMAGES WHATSOEVER
 *  (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
 *  BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER
 *  PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.
 *
 *  Freescale Semiconductor assumes no responsibility for the
 *  maintenance and support of this software
 ********************************************************************/
