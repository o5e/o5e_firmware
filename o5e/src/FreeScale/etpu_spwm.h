/**************************************************************************
 * FILE NAME: $RCSfile: etpu_spwm.h,v $      COPYRIGHT (c) FREESCALE 2005 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the eTPU Synchronised*
 * Pulse Width Modulation (SPWM) API.                                     *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_spwm.h,v $
 * Revision 2.0  2006/01/13 11:22:09  r47354
 * no message
 *
 * Revision 1.3  2006/01/11 17:00:39  r47354
 * Change to MasterRisingEdgePtr
 *
 * Revision 1.2  2005/12/22 14:01:57  r47354
 * Change MasterNextRisingEdgePtr to MasterFrameEdgePtr
 * Rename fs_etpu_spwm_config_master to fs_etpu_spwm_init_master for GCT compliance.
 *
 * Revision 1.1  2005/12/07 15:27:25  r47354
 * *** empty log message ***
 *
 **************************************************************************/
#include "etpu_spwm_auto.h"   /* Auto generated header file for eTPU SPWM */

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/* SPWM channel configuration : master */
int32_t fs_etpu_spwm_init_master ( uint8_t channel,
                                     uint32_t freq,
                                     uint16_t duty, 
                                     uint8_t  timebase,
                                     uint32_t timebase_freq,
                                     uint8_t  reference_mode,
                                     uint32_t *reference_ptr,
                                     uint8_t  INT_DMA_on_active_edge,
                                     uint32_t link1,
                                     uint32_t link2
																   );
                                  
/* SPWM channel run : master */
void fs_etpu_spwm_run_master ( uint8_t channel,
                               uint8_t priority );

/* SPWM channel initialization : slave */
int32_t fs_etpu_spwm_init_slave ( uint8_t  channel,
																	uint8_t  priority,
                                  uint32_t freq,
                                  uint16_t duty,                                  
                                  uint32_t delay,
                                  uint8_t  timebase, 
                                  uint32_t timebase_freq,
                                  uint32_t *MasterRisingEdgePtr
																);

/* SPWM duty cycle update */
void fs_etpu_spwm_duty( uint8_t channel, 
                        uint16_t duty);

/* SPWM duty cycle and freq update */
int32_t fs_etpu_spwm_update_master( uint8_t channel,
                                    uint32_t freq,
                                    uint16_t duty,
                                    uint32_t timebase_freq
                                    );

/* SPWM information */
uint32_t fs_etpu_spwm_get_freq_master( uint8_t channel, 
                                       uint32_t timebase_freq
                                     );


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
 *
 ********************************************************************/





