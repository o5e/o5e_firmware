/**************************************************************************
 * FILE NAME: $RCSfile: etpu_ppa.h,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for ETPU Pulse or Period *
 * Accumulator function (PPA) API                                         *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_ppa.h,v $
 * Revision 1.1  2004/12/09 14:38:31  r47354
 * Updates as per QOM API rel_2_1
 *
 *........................................................................*
 * 0.1   G. Emerson  8/Sep/04  Initial version.                           *
 **************************************************************************/


/**************************************************************************/
#include "etpu_ppa_auto.h"		/*Auto generated header file for eTPU PPA */

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/* PPA channel initialization */
int32_t fs_etpu_ppa_init( uint8_t channel, uint8_t priority, 
                          uint8_t timebase,uint8_t mode, uint32_t max_count, 
                          uint32_t sample_time );
                       
                       
uint8_t fs_etpu_ppa_get_accumulation( uint8_t channel, uint32_t *result );

uint8_t fs_etpu_ppa_get_immediate( uint8_t channel, uint32_t *result, 
                                   uint32_t *current_count );
                      
/* TPU3 PPA API compatible functions to be added */