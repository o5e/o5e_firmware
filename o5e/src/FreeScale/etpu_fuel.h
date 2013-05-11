/*******************************************************************************
* FILE NAME: $RCSfile: etpu_fuel.h,v $
*                                     COPYRIGHT (c) Freescale Semiconductor 2008
*                                               All Rights Reserved
* DESCRIPTION:
* This file contains the prototypes and definitions for the eTPU Function
* FUEL API.
*===============================================================================
* ORIGINAL AUTHOR: Michal Princ (prnm001)                                
* $Log: etpu_fuel.h,v $
* Revision 1.6  2009/01/20 12:44:45  r54529
* Added load calculation for GCT
*
* Revision 1.5  2009/01/14 14:06:31  prnm001
* CAM_chan -> cam_chan
* ms -> us
* function descriptions modified
*
* Revision 1.4  2008/11/27 07:25:36  prnm001
* void in fs_etpu_fuel_get_sum_of_injection_time
*
* Revision 1.3  2008/11/10 15:26:43  prnm001
* Enable to set injection time of all FUEL channels in the fuel init API functions
*
* Revision 1.2  2008/09/29 13:02:15  prnm001
* before testing
*
* Revision 1.1  2008/06/23 08:16:04  prnm001
* Initial version
*
*
*******************************************************************************/

#ifndef _FS_ETPU_FUEL_H_
#define _FS_ETPU_FUEL_H_

#include "etpu_fuel_auto.h"  /* Fuel function auto generated header file */
#include "etpu_cam_auto.h"	 /* Cam function auto generated header file */

/******************************************************************************
* Constants
******************************************************************************/

/******************************************************************************
* Macros
******************************************************************************/
/* polarity options */
#define FS_ETPU_FUEL_PULSE_HIGH FS_ETPU_FUEL_FM0_ACTIVE_HIGH /* Pulse active high */
#define FS_ETPU_FUEL_PULSE_LOW  FS_ETPU_FUEL_FM0_ACTIVE_LOW  /* Pulse active low */

/******************************************************************************
* Types
******************************************************************************/
#include "etpu_util.h"          /* Utility routines for working with the eTPU */

/******************************************************************************
* Global function prototypes
******************************************************************************/
/* Fuel channels initialization */
int32_t fs_etpu_fuel_init_3cylinders(uint8_t  channel_1,
                                     uint8_t  channel_2,
                                     uint8_t  channel_3,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t cylinder_offset_angle_2,
                                     uint24_t cylinder_offset_angle_3,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t injection_time_us_2,
                                     uint24_t injection_time_us_3,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us);
                                     
int32_t fs_etpu_fuel_init_4cylinders(uint8_t  channel_1,
                                     uint8_t  channel_2,
                                     uint8_t  channel_3,
                                     uint8_t  channel_4,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t cylinder_offset_angle_2,
                                     uint24_t cylinder_offset_angle_3,
                                     uint24_t cylinder_offset_angle_4,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t injection_time_us_2,
                                     uint24_t injection_time_us_3,
                                     uint24_t injection_time_us_4,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us);

int32_t fs_etpu_fuel_init_6cylinders(uint8_t  channel_1,
                                     uint8_t  channel_2,
                                     uint8_t  channel_3,
                                     uint8_t  channel_4,
                                     uint8_t  channel_5,
                                     uint8_t  channel_6,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t cylinder_offset_angle_2,
                                     uint24_t cylinder_offset_angle_3,
                                     uint24_t cylinder_offset_angle_4,
                                     uint24_t cylinder_offset_angle_5,
                                     uint24_t cylinder_offset_angle_6,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t injection_time_us_2,
                                     uint24_t injection_time_us_3,
                                     uint24_t injection_time_us_4,
                                     uint24_t injection_time_us_5,
                                     uint24_t injection_time_us_6,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us);


/* Write parameter fucntions */
int32_t fs_etpu_fuel_set_injection_time(uint8_t  channel,
                                        uint24_t injection_time_us);

int32_t fs_etpu_fuel_set_drop_dead_angle(uint8_t  channel,
                                         uint24_t drop_dead_angle);

int32_t fs_etpu_fuel_set_normal_end_angle(uint8_t  channel,
                                          uint24_t normal_end_angle);

int32_t fs_etpu_fuel_set_recalc_offset_angle(uint8_t  channel,
                                             uint24_t recalc_offset_angle);

int32_t fs_etpu_fuel_set_compensation_time(uint8_t  channel,
                                           uint24_t compensation_time_us);

int32_t fs_etpu_fuel_set_minimum_injection_time(uint8_t  channel,
                                                uint24_t minimum_injection_time_us);

int32_t fs_etpu_fuel_set_minimum_off_time(uint8_t  channel,
                                          uint24_t minimum_off_time_us);

int32_t fs_etpu_fuel_switch_off(uint8_t channel);
int32_t fs_etpu_fuel_switch_on(uint8_t channel);


/* Read parameter fucntions */
uint24_t fs_etpu_fuel_get_sum_of_injection_time(void);

uint24_t fs_etpu_fuel_get_CPU_real_injection_time(uint8_t channel);

/*******************************************************************************
* Information for eTPU Graphical Configuration Tool
*******************************************************************************/
/* full function name: Fuel */
/* channel assignment when (fs_etpu_fuel_init_3cylinders: default); load: fuel
   FUEL_1 = channel_1
   FUEL_2 = channel_2
   FUEL_3 = channel_3
*/
/* channel assignment when (fs_etpu_fuel_init_4cylinders: default); load: fuel
   FUEL_1 = channel_1
   FUEL_2 = channel_2
   FUEL_3 = channel_3
   FUEL_4 = channel_4
*/
/* channel assignment when (fs_etpu_fuel_init_6cylinders: default); load: fuel
   FUEL_1 = channel_1
   FUEL_2 = channel_2
   FUEL_3 = channel_3
   FUEL_4 = channel_4
   FUEL_5 = channel_5
   FUEL_6 = channel_6
*/
/* load expression fuel
var engine_speed { 0..20000 } [ rpm ]
 
rev_360_base = 100*(engine_speed/60)/etpu_clock_freq
rev_720_base = rev_360_base/2  

cycles = (118 + 88 + 34 + 22 + 44 + 28 + 22)

FUEL_1 = cycles*rev_720_base
FUEL_2 = cycles*rev_720_base
FUEL_3 = cycles*rev_720_base
FUEL_4 = cycles*rev_720_base
FUEL_5 = cycles*rev_720_base
FUEL_6 = cycles*rev_720_base
*/
#endif /*__FS_ETPU_FUEL_H_ */

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
