/**************************************************************************
 * FILE NAME: $RCSfile: etpu_uart.c,v $      COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the ETPU UART API                                   *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_uart.c,v $
 * Revision 1.3  2004/12/10 08:44:38  r47354
 * - Changes to remove etpu_config.h from builds.
 * - Formatting changes.
 *
 * Revision 1.2  2004/12/09 16:34:52  r47354
 * Update disclaimer.
 *
 * Revision 1.1  2004/12/09 16:25:58  r47354
 * No changes to code, just updated for CVS and checked into CVS.
 *
 *........................................................................*
 * 1.0		V. GOLER      01/JUL/04	Initial Version of Function	            *
 * 1.1		V. GOLER      30/JUL/04 Corrected names of functions	          *
 * 1.2		V. GOLER      03/AUG/04 Corrected typos			                    *
 * 2.0		V. GOLER      18/AUG/04 Combined transmit and receive 		      *
 * 		                            into one function and the receive       *
 *            		                function now returns the data           *
/**************************************************************************/
#include "etpu_util.h"          /* Utility routines for working eTPU */
#include "etpu_uart.h"          /* eTPU SPI API header */
extern uint32_t fs_etpu_data_ram_start;

/************************************************************************
FUNCTION:		fs_etpu_uart_tx_rx_init
************************************************************************
Purpose:		To initialize a channel to function as a UART transmitter and 
            receiver
Input Notes:	This function has 7 parameters:

						tx_channel - 	This is the channel number of the transmitter.
                    			0-31 for ETPU_A and 64-95 for ETPU_B.

						rx_channel - 	This is the channel number of the receiver.
                    			0-31 for ETPU_A and 64-95 for ETPU_B.

						priority   - 	This is the priority to assign to the channel.
			        						This parameter should be assigned a value of:
			        						ETPU_PRIORITY_HIGH, ETPU_PRIORITY_MIDDLE or
		          						ETPU_PRIORITY_LOW.

						 baud_rate - 	Baud rate is a measure of the number of times per
						              second a signal in a communications channel varies,
						              or makes a transition between states (states being
						              frequencies, voltage levels, or phase angles). One
						              baud is one such change. Thus, a 300-baud modem's
						              signal changes state 300 times each second, while
						              a 600- baud modem's signal changes state 600 
						              times per second. The baud_rate value is the 
						              number of TCR1 counts per bit time, and is 
						              calculated by the following equation:

                          # Timer Count Register1 (TCR1) counts / second
                          -------------------------------------------------
                          number of transitions (baud)/ second
                           
                           
      bits_per_data_word -	This is the number of bits to be transmitted in 
                            one data word.  This bits_per_data_word commonly 
                            has a value of eight, because most serial 
                            protocols use 8-bit words.
      
                  parity - 	This is the desired parity.  This parameter 
                            should be assigned a value of 
                            FS_ETPU_UART_NO_PARITY,  FS_ETPU_UART_ODD_PARITY,
                            or FS_ETPU_UART_EVEN_PARITY. The TPU baud rates
                            for the UART function are defined in tpu_uart.h.
      
           timebase_freq -	This is the frequency of the selected timebase.
      			                The range of this is the same as the range of the
      			                timebase frequency on the device.  
      			                This parameter is a uint32_t.
                            
RETURNS NOTES: 		   Error code if channel could not be initialized. Error code that
                 	   can be returned are: FS_ETPU_ERROR_MALLOC , FS_ETPU_ERROR_FREQ
WARNING      		   : *This function does not configure the pin only the eTPU. In a
                  	   system a pin may need to be configured to select the eTPU.
******************************************************************************/

int32_t fs_etpu_uart_init (uint8_t tx_channel, uint8_t rx_channel,
                   uint8_t priority, uint32_t baud_rate,
                   uint8_t bits_per_data_word, uint8_t parity,
                   uint32_t timebase_freq)
{
    uint32_t *pba;              /* parameter base address for channel        */
    uint32_t chan_match_rate;   /* value to be used for each bit time  */
    uint8_t *pba8;

    /* Disable channels to assign function safely */
    fs_etpu_disable (rx_channel);
    fs_etpu_disable (tx_channel);

/* initialize receive channel  */
    if (eTPU->CHAN[rx_channel].CR.B.CPBA == 0)
      {
/* get parameter RAM
   number of parameters passed from eTPU C code */
          pba = fs_etpu_malloc (FS_ETPU_UART_NUM_PARMS);

          if (pba == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else
      {   /*set pba to what is in the CR register */
          pba = fs_etpu_data_ram (rx_channel);
      }

    pba8 = (uint8_t *) pba;

    /* Determine bit time of serially transmitted data  */
    chan_match_rate = timebase_freq / baud_rate;
    /* integer division truncates, so any fractional part is discarded */

    if ((chan_match_rate == 0) || (chan_match_rate > 0x007FFFFF))
        return (FS_ETPU_ERROR_FREQ);

    /* write match_rate calculated from time base 
       frequency and desired baud rate      */
    *(pba + ((FS_ETPU_UART_MATCH_RATE_OFFSET - 1) >> 2)) = chan_match_rate;

    /* configure the number of bits per data word.
       this number represents only the number of 
       data bits and does not include start,
       stop, or parity bits. */
    *(pba8 + (FS_ETPU_UART_BITS_PER_DATA_WORD_OFFSET)) = bits_per_data_word;

    /* write channel configuration register */
    eTPU->CHAN[rx_channel].CR.R = (priority << 28) +
        (FS_ETPU_UART_TABLE_SELECT << 24) +
        (FS_ETPU_UART_FUNCTION_NUMBER << 16) +
        (((uint32_t) pba - fs_etpu_data_ram_start) >> 3);

    /* write FM (function mode) bits in channel Status Control Register (SCR) */
    eTPU->CHAN[rx_channel].SCR.R = parity;

    /* write hsr to start channel running */
    eTPU->CHAN[rx_channel].HSRR.R = FS_ETPU_UART_RX_INIT;

/* initialize transmit channel */
    if (eTPU->CHAN[tx_channel].CR.B.CPBA == 0)
      {
/* get parameter RAM
        number of parameters passed from eTPU C code */
          pba = fs_etpu_malloc (FS_ETPU_UART_NUM_PARMS);

          if (pba == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else
      {                         /*set pba to what is in the CR register */
          pba = fs_etpu_data_ram (tx_channel);
      }

    pba8 = (uint8_t *) pba;

    /* write match_rate calculated from time base frequency 
       and desired baud rate        */
    *(pba + ((FS_ETPU_UART_MATCH_RATE_OFFSET - 1) >> 2)) = chan_match_rate;

    /* set the TDRE flag to indicate that the transmit data reg is empty*/
    *(pba + ((FS_ETPU_UART_TX_RX_DATA_OFFSET - 1) >> 2)) = 0x800000;
    /* a one in the MSB indicates that the data register is empty */

    /* configure the number of bits per data word.This number represents 
       only the number of data bits and does not include start, stop, or 
       parity bits. */
    *(pba8 + (FS_ETPU_UART_BITS_PER_DATA_WORD_OFFSET)) = bits_per_data_word;

    /* write channel configuration register */
    eTPU->CHAN[tx_channel].CR.R = (priority << 28) +
        (FS_ETPU_UART_TABLE_SELECT << 24) +
        (FS_ETPU_UART_FUNCTION_NUMBER << 16) +
        (((uint32_t) pba - fs_etpu_data_ram_start) >> 3);

    /* write FM (function mode) bits in channel Status Control Register (SCR) */
    eTPU->CHAN[tx_channel].SCR.R = parity;

    /* write hsr to start channel running */
    eTPU->CHAN[tx_channel].HSRR.R = FS_ETPU_UART_TX_INIT;

    return (0);
};

/************************************************************************
FUNCTION:		fs_etpu_uart_write_transmit_data
************************************************************************
Purpose:		To write data to eTPU UART transmitter channel for data to be 
            Serially shifted out

Input Notes: This function has 2 parameters

            channel - This is the channel number.
                      0-31 for ETPU_A and 64-95 for ETPU_B.
      
      transmit_data - This the actual data to be transmitted.  
                      Up to 22 bits of data per data word can be transmitted.
      
******************************************************************************/

void fs_etpu_uart_write_transmit_data (uint8_t tx_channel, 
                                       uint32_t transmit_data)
{
    uint32_t *pba;              /* parameter base address for channel   */

    pba = fs_etpu_data_ram (tx_channel);

/* write the transmit data register	*/
    transmit_data &= 0x7fffff;
    *(pba + ((FS_ETPU_UART_TX_RX_DATA_OFFSET - 1) >> 2)) = transmit_data;
/* clear MSB to indicate valid data to transmit	*/

}

/************************************************************************
FUNCTION:		fs_etpu_uart_read_receive_data
************************************************************************
Purpose:		To read received data from eTPU UART receive channel,
                and to optionally check for parity and or framing errors

Input Notes:    This function has 2 parameters

                channel - 		This is the channel number.
                              0-31 for ETPU_A and 64-95 for ETPU_B.
                
                
              *rx_error - This is a pointer to a location to contain an error 
                          code
                    A value of O - no error
                           0x80  - parity error
                           0x40  - framing error
                           0xc0  - both parity and framing error
                will be placed in the location pointed to by pointer rx_error

A parity error is detected when the odd/even number of ones in the data does
not match the programmed (expected) value.  A framing error occurs when the 
UART function determines that a stop bit is low instead of high.  All errors
are only valid for each received data word.

The function returns the received data right justified.

******************************************************************************/

int32_t fs_etpu_uart_read_receive_data (uint8_t rx_channel, uint8_t * rx_error)
{
    uint32_t *pba;  /* parameter base address for channel */
    uint8_t *pba8;  /* paramter base address for channel */
    uint32_t receive_data;

    pba = fs_etpu_data_ram (rx_channel);
    pba8 = (uint8_t *) pba;

    /* place error code in location pointed to by *rx_error pointer */
    *rx_error = *(pba8 + FS_ETPU_UART_RX_ERROR_OFFSET);

    /*right justify the data */
    receive_data = *(pba + ((FS_ETPU_UART_TX_RX_DATA_OFFSET - 1) >> 2));
    /* get received data  */
    receive_data &= 0x00ffffff;
    return (receive_data >>
            (23 - *(pba8 + FS_ETPU_UART_BITS_PER_DATA_WORD_OFFSET)));
    /* right justify received data */
}

/*********************************************************************
 *
 * Copyright:
 *  Freescale Semiconductor, INC. All Rights Reserved.
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
