/**************************************************************************
 * FILE NAME: $RCSfile: etpu_app_eng_pos.h,v $                            *
 *                                           COPYRIGHT (c) FREESCALE 2007 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the eTPU automotive  *
 * synchronisation functions.                                             *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_app_eng_pos.h,v $
 * Revision 1.7  2009/01/20 12:45:27  r54529
 * Corrected load calculation for GCT
 *
 * Revision 1.6  2009/01/14 11:08:52  r47354
 * Added load calculation for GCT
 *
 * Revision 1.5  2008/11/26 15:29:36  r47354
 * Add type for parameter in speed API.
 *
 * Revision 1.4  2008/11/13 10:48:57  r47354
 * Added fs_etpu_eng_pos_get_engine_speed function.
 *
 * Revision 1.3  2008/08/28 13:22:50  r47354
 * fs_etpu_eng_pos_insert_tooth now has host_asserted_tooth_count parameter.
 *
 * Revision 1.2  2008/06/06 09:53:27  r47354
 * Major rework based on etpuc code changes and review.
 *
 * Revision 1.1  2008/01/21 13:49:05  r47354
 * Initial revision
 *
 *
 **************************************************************************/
#include "etpu_cam_auto.h"   /* Auto generated header file for eTPU cam   */
#include "etpu_crank_auto.h" /* Auto generated header file for eTPU crank */

/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/* eng_pos channel initialisation */

int32_t fs_etpu_app_eng_pos_init ( uint8_t cam_channel,
                                   uint8_t cam_priority,
                                   uint8_t cam_edge_polarity,
                                  uint32_t cam_angle_window_start,
                                  uint32_t cam_angle_window_width,
                                   uint8_t crank_channel,
                                   uint8_t crank_priority,
                                   uint8_t crank_edge_polarity,
                                   uint8_t crank_number_of_physical_teeth,
                                   uint8_t crank_number_of_missing_teeth,
                                   uint8_t crank_blank_tooth_count,
                                  uint32_t crank_tcr2_ticks_per_tooth,
                                ufract24_t crank_windowing_ratio_normal, 
                                ufract24_t crank_windowing_ratio_after_gap, 
                                ufract24_t crank_windowing_ratio_across_gap, 
                                ufract24_t crank_windowing_ratio_timeout, 
                                ufract24_t crank_gap_ratio, 
                                  uint32_t crank_blank_time_ms,
                                  uint32_t crank_first_tooth_timeout_us,
                                  uint32_t crank_link_1,
                                  uint32_t crank_link_2,
                                  uint32_t crank_link_3,
                                  uint32_t crank_link_4,
                                  uint32_t tcr1_timebase_freq );

/* read parameter functions */
uint32_t fs_etpu_eng_pos_get_cam_edge_angle();
int8_t fs_etpu_eng_pos_get_tooth_number();
int32_t fs_etpu_eng_pos_get_tooth_time();
int32_t fs_etpu_eng_pos_get_engine_speed(uint32_t tcr1_timebase_freq);

/* read status functions */
int8_t fs_etpu_eng_pos_get_engine_position_status();
int8_t fs_etpu_eng_pos_get_cam_status();
int8_t fs_etpu_eng_pos_get_crank_status();

/* read error status functions */
int8_t fs_etpu_eng_pos_get_cam_error_status();
int8_t fs_etpu_eng_pos_get_crank_error_status();

/* clear error status functions */
int32_t fs_etpu_eng_pos_clear_cam_error_status();
int32_t fs_etpu_eng_pos_clear_crank_error_status();

/* update parameter functions */
int32_t fs_etpu_eng_update_cam_window (uint32_t cam_angle_window_start, 
                                       uint32_t cam_angle_window_width);
                                       
/* set windowing ratio functions */
int32_t fs_etpu_eng_pos_set_wr_normal( ufract24_t ratio );
int32_t fs_etpu_eng_pos_set_wr_after_gap( ufract24_t ratio );
int32_t fs_etpu_eng_pos_set_wr_across_gap( ufract24_t ratio );
int32_t fs_etpu_eng_pos_set_wr_timeout( ufract24_t ratio );

/* advanced functions */
int32_t fs_etpu_eng_pos_insert_tooth( uint8_t host_asserted_tooth_count );
int32_t fs_etpu_eng_pos_adjust_angle( int24_t angle_adjust );

/* debug functions */
int32_t fs_etpu_eng_pos_get_engine_cycle_0_tcr2_count();
int32_t fs_etpu_eng_pos_set_tooth_number ( uint8_t tooth_number );

/**************************************************************************
*        Information for eTPU Graphical Configuration Tool 
***************************************************************************/
/* eTPU functions used: CAM, CRANK */
/* full function name: Engine Position (CAM and CRANK channels) */ 
/* channel assignment; load: eng_pos
   CAM = cam_channel; priority=cam_priority
   CRANK = crank_channel; priority=crank_priority
*/
/* load expression eng_pos
var engine_speed { 0..20000 } [ rpm ]
 
rev_360_base = 100*(engine_speed/60)/etpu_clock_freq
rev_720_base = rev_360_base/2  

CAM = (65)*rev_720_base
CRANK = (136 + (crank_number_of_physical_teeth - 2)*118 + 154)*rev_360_base
*/