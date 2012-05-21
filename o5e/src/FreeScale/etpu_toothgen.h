/*******************************************************************************
* FILE NAME: etpu_ToothGen.h        COPYRIGHT (c) Freescale Semiconductor 2006
*                                               All Rights Reserved
* DESCRIPTION:
* This file contains the prototypes and definitions for the eTPU Function
* Tooth Generator (ToothGen) API.
*===============================================================================
* REV      AUTHOR      DATE        DESCRIPTION OF CHANGE
* ---   -----------  ----------    ---------------------
* 0.0   M  Li        19/Jan/04     Initial creation
* 0.1   S. Mihalik   07/Jun/06     Changed to Freescale typedefs
* 1.0   M. Brejl     07/Sep/06     Complete rework. New features.
* 1.1   M. Brejl     28/Mar/08     DoxyGen keywords removed.
* 1.2   M. Brejl     03/Apr/08     Some parameters changed.
* 1.3   M. Brejl     13/May/08     2nd missing tooth and tooth in gap added.
* 1.4   M. Brejl     27/Nov/08     use of ufract24_t.
* *******************************************************************************/
#ifndef _FS_ETPU_TOOTHGEN_H_
#define _FS_ETPU_TOOTHGEN_H_

#include "etpu_toothgen_auto.h"  /* Auto generated header file */

/******************************************************************************
* Constants
******************************************************************************/

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Types
******************************************************************************/
#include "etpu_util.h"          /* Utility routines for working with the eTPU */
/******************************************************************************
* Global variables
******************************************************************************/

/******************************************************************************
* Global function prototypes
******************************************************************************/
/* ToothGen channels (cam and crank) initialization */
int32_t fs_etpu_toothgen_init( uint8_t channel_crank,
                               uint8_t channel_cam,
                               uint8_t priority,
                               uint8_t crank_number_of_physical_teeth,
                               uint8_t crank_number_of_missing_teeth,
                            ufract24_t tooth_duty_cycle,
                               uint8_t tooth_number,
                              uint32_t engine_speed_rpm,
                              uint32_t tcr1_timebase_freq,
                               uint8_t cam_start,
                               uint8_t cam_stop );

/* Addjust parameter to generate acc/deceleration tooth pattern profile */
int32_t fs_etpu_toothgen_adj( uint8_t channel_crank,
                           ufract24_t accel_rate,
                             uint32_t target_engine_speed_rpm,
                             uint32_t tcr1_timebase_freq );

/* Set error mimic - missing crank tooth */
int32_t fs_etpu_toothgen_set_missing_tooth( uint8_t channel_crank,
                                            uint8_t error_type,
                                            uint8_t missing_tooth_number_1,
                                            uint8_t missing_tooth_number_2 );

/* Clear error mimic - missing crank tooth */
void fs_etpu_toothgen_clear_missing_tooth( uint8_t channel_crank );

/* Set error mimic - crank noise */
int32_t fs_etpu_toothgen_set_crank_noise( uint8_t channel_crank,
                                          uint8_t noise_tooth_number,
                                         uint24_t noise_pulse_shift,
                                         uint24_t noise_pulse_width );

/* Clear error mimic - crank noise */
void fs_etpu_toothgen_clear_crank_noise( uint8_t channel_crank );

/* Set error mimic - missing cam pulse */
void fs_etpu_toothgen_set_missing_cam_pulse( uint8_t channel_cam );

/* Clear error mimic - missing cam pulse */
void fs_etpu_toothgen_clear_missing_cam_pulse( uint8_t channel_cam );

/* Set error mimic - cam noise */
int32_t fs_etpu_toothgen_set_cam_noise( uint8_t channel_cam,
                                        uint8_t noise_tooth_number,
                                       uint24_t noise_pulse_shift,
                                       uint24_t noise_pulse_width );

/* Clear error mimic - cam noise */
void fs_etpu_toothgen_clear_cam_noise( uint8_t channel_cam );

/* Set error mimic - tooth in gap */
void fs_etpu_toothgen_set_tooth_in_gap( uint8_t channel_cam );

/* Clear error mimic - tooth in gap */
void fs_etpu_toothgen_clear_tooth_in_gap( uint8_t channel_cam );

/* Update all kinds of error mimics at once */
int32_t fs_etpu_toothgen_set_error_mimics( uint8_t channel_crank,
                                           uint8_t error_list,
                                           uint8_t missing_tooth_number_1,
                                           uint8_t missing_tooth_number_2,
                                           uint8_t crank_noise_tooth_number,
                                          uint24_t crank_noise_pulse_shift,
                                          uint24_t crank_noise_pulse_width,
                                           uint8_t cam_noise_tooth_number,
                                          uint24_t cam_noise_pulse_shift,
                                          uint24_t cam_noise_pulse_width );


/*******************************************************************************
* Information for eTPU Graphical Configuration Tool
*******************************************************************************/
/* full function name: ToothGen - Tooth Generator */
/* channel assignment; load: toothgen
   Crank = channel_crank
   Cam = channel_cam
*/
/* load expression toothgen
rev_base = 100*engine_speed_rpm/(60*etpu_clock_freq)
pulse_base = (crank_number_of_physical_teeth + crank_number_of_missing_teeth)*rev_base
Crank = (54+102)*pulse_base
Cam = 0
*/

#endif /*__FS_ETPU_TOOTHGEN_H_ */

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
