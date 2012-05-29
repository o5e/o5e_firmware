
/****************************************************************
 * WARNING this file is automatically generated DO NOT EDIT IT! *
 *                                                              *
 * FILE NAME: etpu_uart_auto.h     COPYRIGHT (c) Freescale 2004 *
 *                                      All Rights Reserved     *
 * This file generated by:                                      *
 * $RCSfile: etpuc_uart.c,v $ $Revision: 1.3 $
 *                                                              *
 * This file provides an interface between eTPU code and CPU    *
 * code. All references to the UART function should be made with*
 * information in this file. This allows only symbolic          *
 * information to be referenced which allows the eTPU code to be*
 * optimized without effecting the CPU code.                    *
 ****************************************************************/
#ifndef _ETPU_UART_AUTO_H_
#define _ETPU_UART_AUTO_H_

/* Function Configuration Information */
#define FS_ETPU_UART_FUNCTION_NUMBER  9 
#define FS_ETPU_UART_TABLE_SELECT 1 
#define FS_ETPU_UART_NUM_PARMS 0x0010 

/* Host Service Request Definitions */
#define FS_ETPU_UART_TX_INIT 4 
#define FS_ETPU_UART_RX_INIT 7

/* Function Mode Bit Definitions - polarity options */
#define FS_ETPU_UART_NO_PARITY 0 
#define FS_ETPU_UART_EVEN_PARITY 0  + 2 
#define FS_ETPU_UART_ODD_PARITY 1  + 2 

/* Parameter Definitions */
#define FS_ETPU_UART_MATCH_RATE_OFFSET  0x0001
#define FS_ETPU_UART_TX_RX_DATA_OFFSET  0x0009
#define FS_ETPU_UART_BITS_PER_DATA_WORD_OFFSET  0x000C
#define FS_ETPU_UART_ACTUAL_BIT_COUNT_OFFSET  0x0000
#define FS_ETPU_UART_SHIFT_REG_OFFSET  0x0005
#define FS_ETPU_UART_PARITY_TEMP_OFFSET  0x0004
#define FS_ETPU_UART_RX_ERROR_OFFSET  0x0008

#endif /* _ETPU_UART_AUTO_H_ */
