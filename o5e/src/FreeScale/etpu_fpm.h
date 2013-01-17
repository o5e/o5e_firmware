/**************************************************************************
 * FILE NAME: $RCSfile: etpu_fpm.h,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the ETPU Frequency   *
 * Pulse Measurement (FPM) API                                            *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_fpm.h,v $
 * Revision 1.1  2005/01/13 13:35:20  r47354
 * Initial revision.
 *
 *........................................................................*
 *************************************************************************/
#include "etpu_fpm_auto.h"  /* Auto generated header file for eTPU FPM */ 

/*--------------------------------------------------------------------+
|                           Function Prototypes                       |
+--------------------------------------------------------------------*/

/* FPM channel initialization */
int16_t fs_etpu_fpm_init( uint8_t channel, 
                       uint8_t priority, 
                       uint8_t mode, 
                       uint8_t edge, 
                       uint8_t timebase, 
                       uint32_t window_size);
          
/* FPM update window size */
void fs_etpu_fpm_update_window_size( uint8_t channel, uint32_t window);

/* FPM read count */
uint32_t fs_etpu_fpm_get_count( uint8_t channel);

/* FPM read frequency */
uint32_t fs_etpu_fpm_get_freq( uint8_t channel, uint32_t timebase_freq);

/*--------------------------------------------------------------------+
|              Function Prototypes for TPU3 compatability             |
+--------------------------------------------------------------------*/


///* FQM channel initialization */
//void tpu_fqm_init(struct TPU3_tag *tpu, UINT8 channel, UINT8 priority, 
//				UINT8 mode, UINT8 edge, UINT8 timebase, UINT16 wind_sz);
//
///* FQM channel update window */
//void	tpu_fqm_update_window_size(struct TPU3_tag *tpu,   
//					UINT8 channel, UINT16 wind_sz);
//
///* FQM channel get pulse */
//UINT16	tpu_fqm_get_pulse(struct TPU3_tag *tpu, UINT8 channel);

