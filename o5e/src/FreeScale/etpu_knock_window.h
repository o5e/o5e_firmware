/**************************************************************************
 * FILE NAME: $RCSfile: etpu_knock_window.h,v $                           *
 *                                           COPYRIGHT (c) FREESCALE 2008 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the eTPU automotive  *
 * knock window output function.                                          *
 *========================================================================*
 * ORIGINAL AUTHOR: David Paterson (r43562)                               *
 * $Log: etpu_knock_window.h,v $
 * Revision 1.3  2009/01/20 12:41:47  r54529
 * Added load calculation for GCT
 *
 * Revision 1.2  2008/10/29 16:21:49  r43562
 * Updated number_of_windows to be uint8 (previously uint24) and updated INPUT NOTES in comments above functions.
 *
 * Revision 1.1  2008/06/25 08:00:05  r43562
 * Initial version.
 *
 *
 **************************************************************************/
#include "etpu_knock_window_auto.h" /* Auto generated header file for     */
																											      /* eTPU knock */
#include "etpu_cam_auto.h"     /* Auto generated header file for eTPU CAM */
/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

int32_t fs_etpu_knock_window_init ( uint8_t channel,
                             uint8_t priority,
                             uint8_t number_of_windows,
                             uint8_t edge_polarity,
                             uint8_t edge_interrupt,                            
                             uint8_t cam_chan,
                             uint32_t *knock_window_angle_table
                            );

int32_t fs_etpu_knock_window_update ( uint8_t channel,
                                      uint8_t cam_channel, 
                                      uint8_t update_window_number, 
                                      uint32_t angle_open_new,
                                      uint32_t angle_width_new );

/**************************************************************************
*        Information for eTPU Graphical Configuration Tool 
***************************************************************************/
/* full function name: Knock Window */ 
/* channel assignment; load: knock_window
   KNOCK_WINDOW = channel
*/
/* load expression knock_window
var engine_speed { 0..20000 } [ rpm ]
 
rev_360_base = 100*(engine_speed/60)/etpu_clock_freq
rev_720_base = rev_360_base/2  

KNOCK_WINDOW = (number_of_windows*(10+48) + 14)*rev_720_base
*/