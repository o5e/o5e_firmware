/**************************************************************************
 * FILE NAME: $RCSfile: etpu_uart.h,v $      COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the ETPU UART API    *
 *========================================================================*
 * ORIGINAL AUTHOR: Mario Perez (r2494c)                                  *
 * $Log: etpu_uart.h,v $
 * Revision 1.1  2004/12/09 16:25:58  r47354
 * No changes to code, just updated for CVS and checked into CVS.
 *
 *........................................................................*
 * 0.1   V.   Goler  12Jul04 		Initial version.                          *
 **************************************************************************/
 
#include "etpu_uart_auto.h"     /*Auto generated header file for eTPU uart*/

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/
int32_t fs_etpu_uart_init( uint8_t tx_channel, uint8_t rx_channel, uint8_t priority,
                                    uint32_t baud_rate, uint8_t bits_per_data_word,
                                    uint8_t parity, uint32_t timebase_freq);

void fs_etpu_uart_write_transmit_data( uint8_t tx_channel, uint32_t transmit_data);

int32_t fs_etpu_uart_read_receive_data( uint8_t rx_channel, uint8_t *rx_error);


