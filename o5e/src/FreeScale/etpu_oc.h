/**************************************************************************
 * FILE NAME: $RCSfile: etpu_oc.h,v $        COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the ETPU Output      *
 * Compare (OC) API                                                       *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_oc.h,v $
 * Revision 1.3  2004/12/07 14:01:46  r47354
 * Added more comments to fs_etpu_oc_data.
 *
 * Revision 1.2  2004/12/07 13:12:35  r47354
 * Updates as per QOM API rel_2_1
 *
 *........................................................................*
 * 0.1   G. Emerson   21/May/04     Initial version.                      *
 * 0.2                              Updated for new build structure.      *
 * 0.3   G.Emerson  16/July/04 Add Disclaimer                             *
 **************************************************************************/

#include "etpu_qom_auto.h"		/*Auto generated header file for eTPU QOM*/

/**************************************************************************/
/*                            Definitions                                 */
/**************************************************************************/


#define FS_ETPU_OC_INIT_PIN_LOW 5
#define FS_ETPU_OC_INIT_PIN_HIGH 6
#define FS_ETPU_OC_INIT_PIN_NO_CHANGE 7

#define FS_ETPU_OC_MATCH_TCR1_CAPTURE_TCR1 0
#define FS_ETPU_OC_MATCH_TCR2_CAPTURE_TCR2 1

struct offset {
	int timebase:1;
	int :8;
	int offset:22;
	int pin:1;
	} ;


/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/* OC channel initialization */

/* OC channel initialization, immediate mode */
/***************************************************************************/

uint8_t fs_etpu_oc_init_immed( uint8_t channel, uint8_t priority, 
                               struct offset Offset1, struct offset Offset2,
                               uint8_t init_pin);

/* OC channel initialization, reference mode*/

uint8_t fs_etpu_oc_init_ref( uint8_t channel, uint8_t priority, 
                             struct offset Offset1, struct offset Offset2,
                             uint8_t init_pin, uint32_t *ref);
                             
/* OC channel initialization, value mode */

uint8_t fs_etpu_oc_init_value( uint8_t channel, uint8_t priority,
                               struct offset  Offset1, struct offset Offset2,
                               uint8_t init_pin);


/******************************************************************************
FUNCTION     : fs_etpu_oc_data
PURPOSE      : To get the last match time for the 2nd event on an OC channel
INPUTS NOTES : This function has 6 parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
******************************************************************************/
int32_t fs_etpu_oc_data(uint8_t channel);

/*struct OC_results *fs_etpu_oc_data(uint8_t channel); */
/* this function will eventually return a structure containing the times of
   the events; not supported in this release as QOM microcode is being used 
   and doesn't record the first match time */
/* Current etpuc function does however support the match time for the 
   second event; this function returns this vlaue */

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