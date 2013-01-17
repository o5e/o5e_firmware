/***************************************************************************
* FILE NAME: etpu_spark.h                     COPYRIGHT (c) FREESCALE 2009 *
*                                                      All Rights Reserved *
*==========================================================================*
* ORIGINAL AUTHOR: Stan Ostrum                LAST UPDATE: 09/01/05 11:09  *
*                                                                          *
* DESCRIPTION: This file contains the prototypes and definitions for the   *
*              eTPU set2 SPARK function API.                               *
*                                                                          *
****************************************************************************
* REVISION HISTORY:
*
* $Log: etpu_spark.h,v $
* Revision 1.4  2009/01/27 15:27:14  r47354
* Added default init parameters for end angles and dwell times.
* Changed GCT load expression
*
* Revision 1.3  2009/01/20 13:21:55  r54529
* Return data types changed to int32_t.
*
* Revision 1.2  2009/01/20 12:55:32  r54529
* Added load calculation for GCT
*
* Revision 1.1  2009/01/06 15:18:40  ra5622
* Initial release.
*
*
***************************************************************************/

#ifndef _FS_ETPU_SPARK_H_
#define _FS_ETPU_SPARK_H_

#include "etpu_spark_auto.h"    // SPARK function auto generated header file
#include "etpu_cam_auto.h"      // CAM function auto generated header file

/******************************************************************************
* Constants
******************************************************************************/

/******************************************************************************
* Macros
******************************************************************************/

/* polarity options */
#define FS_ETPU_SPARK_ACTIVE_HIGH    FS_ETPU_SPARK_FM0_ACTIVE_HIGH
#define FS_ETPU_SPARK_ACTIVE_LOW     FS_ETPU_SPARK_FM0_ACTIVE_LOW

/******************************************************************************
* Global function prototypes
******************************************************************************/

/* SPARK initialization */

int32_t fs_etpu_spark_init_3cylinders(  uint8_t  spark_channel_1,
                                        uint8_t  spark_channel_2,
                                        uint8_t  spark_channel_3,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint24_t cyl_offset_angle_2,
                                        uint24_t cyl_offset_angle_3,
                                        uint8_t  priority,
                                        uint8_t  polarity,
                                        uint24_t min_dwell_time,
                                        uint24_t max_dwell_time,
                                        uint24_t multi_on_time,
                                        uint24_t multi_off_time,
                                        uint8_t  multi_num_pulses,
                                        uint24_t recalc_offset_angle,
                                        uint24_t init_dwell_time_1,
                                        uint24_t init_dwell_time_2,
                                        uint24_t init_end_angle_1,
                                        uint24_t init_end_angle_2 );
                                        
int32_t fs_etpu_spark_init_4cylinders(  uint8_t  spark_channel_1,
                                        uint8_t  spark_channel_2,
                                        uint8_t  spark_channel_3,
                                        uint8_t  spark_channel_4,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint24_t cyl_offset_angle_2,
                                        uint24_t cyl_offset_angle_3,
                                        uint24_t cyl_offset_angle_4,
                                        uint8_t  priority,
                                        uint8_t  polarity,
                                        uint24_t min_dwell_time,
                                        uint24_t max_dwell_time,
                                        uint24_t multi_on_time,
                                        uint24_t multi_off_time,
                                        uint8_t  multi_num_pulses,
                                        uint24_t recalc_offset_angle,
                                        uint24_t init_dwell_time_1,
                                        uint24_t init_dwell_time_2,
                                        uint24_t init_end_angle_1,
                                        uint24_t init_end_angle_2 );
                                        
int32_t fs_etpu_spark_init_6cylinders(  uint8_t  spark_channel_1,
                                        uint8_t  spark_channel_2,
                                        uint8_t  spark_channel_3,
                                        uint8_t  spark_channel_4,
                                        uint8_t  spark_channel_5,
                                        uint8_t  spark_channel_6,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint24_t cyl_offset_angle_2,
                                        uint24_t cyl_offset_angle_3,
                                        uint24_t cyl_offset_angle_4,
                                        uint24_t cyl_offset_angle_5,
                                        uint24_t cyl_offset_angle_6,
                                        uint8_t  priority,
                                        uint8_t  polarity,
                                        uint24_t min_dwell_time,
                                        uint24_t max_dwell_time,
                                        uint24_t multi_on_time,
                                        uint24_t multi_off_time,
                                        uint8_t  multi_num_pulses,
                                        uint24_t recalc_offset_angle,
                                        uint24_t init_dwell_time_1,
                                        uint24_t init_dwell_time_2,
                                        uint24_t init_end_angle_1,
                                        uint24_t init_end_angle_2 );
                                        
/* SPARK parameter updates */

int32_t fs_etpu_spark_set_dwell_times(  uint8_t  channel,
                                        uint24_t spark1_dwell_time,
                                        uint24_t spark2_dwell_time );

int32_t fs_etpu_spark_set_end_angles(   uint8_t  channel,
                                        uint24_t spark1_end_angle,
                                        uint24_t spark2_end_angle);

int32_t fs_etpu_spark_set_recalc_offset_angle( uint8_t  channel,
                                               uint24_t recalc_offset_angle);

int32_t fs_etpu_spark_set_min_max_dwell_times( uint8_t  channel,
                                               uint24_t min_dwell_time,
                                               uint24_t max_dwell_time);

int32_t fs_etpu_spark_set_multi_pulses( uint8_t  channel,
                                        uint24_t multi_on_time,
                                        uint24_t multi_off_time,
                                        uint8_t  multi_num_pulses);

/*******************************************************************************
* Information for eTPU Graphical Configuration Tool
*******************************************************************************/
/* full function name: Spark */
/* channel assignment when (fs_etpu_spark_init_3cylinders: default); load: spark
   SPARK_1 = spark_channel_1
   SPARK_2 = spark_channel_2
   SPARK_3 = spark_channel_3
*/
/* channel assignment when (fs_etpu_spark_init_4cylinders: default); load: spark
   SPARK_1 = spark_channel_1
   SPARK_2 = spark_channel_2
   SPARK_3 = spark_channel_3
   SPARK_4 = spark_channel_4
*/
/* channel assignment when (fs_etpu_spark_init_6cylinders: default); load: spark
   SPARK_1 = spark_channel_1
   SPARK_2 = spark_channel_2
   SPARK_3 = spark_channel_3
   SPARK_4 = spark_channel_4
   SPARK_5 = spark_channel_5
   SPARK_6 = spark_channel_6
*/
/* load expression spark
var engine_speed { 0..20000 } [ rpm ]
 
rev_360_base = 100*(engine_speed/60)/etpu_clock_freq
rev_720_base = rev_360_base/2  

cycles = if(init_dwell_time_2 == 0, \
           if(multi_num_pulses == 0, \
             44+122 + 84+16+32+116, \
             44+128 + 84+16+32+44 + (multi_num_pulses-1)*(14+28) + 14+118), \
           if(multi_num_pulses == 0, \
             2*(84+16+32+116) + 6, \
             2*(84+16+32+44 + (multi_num_pulses-1)*(14+28) + 14+118) + 6 ))

SPARK_1 = cycles*rev_720_base
SPARK_2 = cycles*rev_720_base
SPARK_3 = cycles*rev_720_base
SPARK_4 = cycles*rev_720_base
SPARK_5 = cycles*rev_720_base
SPARK_6 = cycles*rev_720_base
*/
#endif  //__FS_ETPU_SPARK_H_

/*********************************************************************
 *
 * Copyright:
 *	Freescale Semiconductor, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is retained
 *  without alteration in any modified and/or redistributed versions,
 *  and that such modified versions are clearly identified as such. No
 *  licenses are granted by implication, estoppel or otherwise under
 *  any patents or trademarks of Freescale Semiconductor, Inc. This
 *  software is provided on an "AS IS" basis and without warranty.
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

