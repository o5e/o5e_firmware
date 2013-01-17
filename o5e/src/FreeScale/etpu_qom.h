/**************************************************************************
 * FILE NAME: $RCSfile: etpu_qom.h,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the ETPU Queued      *
 * Output Match (QOM) API                                                 *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_qom.h,v $
 * Revision 1.2  2004/11/11 14:26:28  r47354
 * Tidy up and formatting.
 * Add disclaimer.
 *
 * Revision 1.1  2004/11/09 11:25:44  r47354
 * No changes to code, just updated for CVS and checked into CVS.
 *........................................................................*
 * 0.1   G. Emerson  2/Apr/04  Initial version.                           *
 * 0.2   G. Emerson  28/Apr/04 Remove fs_etpu_qom_send_link               *
 * 0.3                         Update for new build structure.            *
 * 0.4   G. Emerson            Minor change forGCT compatibility.         *
 * 0.5   G. Emerson  1/Nov/04  Remove path in #include                    *
 *                             Remove debug comments                      *
 **************************************************************************/
#include "etpu_qom_auto.h"      /* Auto generated header file for eTPU QOM */

/**************************************************************************/
/*                            Definitions                                 */
/**************************************************************************/

union etpu_events_array {
    uint32_t p;
    struct {
        uint32_t empty:9;
        uint32_t offset:22;
        uint32_t pin:1;
    } b;
};
/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/******************************************************************************
FUNCTION     : etpu_qom_init
PURPOSE      : To initialize an eTPU channel to generate a QOM output.
INPUTS NOTES : This function has 10 parameters:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
              priority - This is the priority to assign to the channel.
                         This parameter should be assigned a value of:
                         ETPU_PRIORITY_HIGH, ETPU_PRIORITY_MIDDLE or
                         ETPU_PRIORITY_LOW.
                  mode - This define the repeat mode for the channel
                         This parameter should be assigned a value of:
                         FS_ETPU_QOM_SINGLE_SHOT,
                         FS_ETPU_QOM_LOOP,
                         FS_ETPU_QOM_CONTINUOUS
                         FS_ETPU_QOM_CONTINUOUS_A
              timebase - This is the timer to use as a reference for the QOM
                         signal. This parameter should be assigned to a value
                         of: ETPU_TCR1 or ETPU_TCR2.
              init_pin - The  state  the pin is driven to at initialisation.
               		       This parameter should be assigned a value of:
                         FS_ETPU_QOM_INIT_PIN_LOW, FS_ETPU_QOM_INIT_PIN_HIGH or
                         FS_ETPU_QOM_INIT_PIN_NO_CHANGE.
                         To initialise the channel to recieve a link the
                         parameter takes the value of :
                         FS_ETPU_QOM_INIT_PIN_LOW_LINK,
                         FS_ETPU_QOM_INIT_PIN_HIGH_LINK,
                         FS_ETPU_QOM_INIT_PIN_NO_CHANGE_LINK.
      first_match_mode - This is the reference mode for the first match
                         This parameter should be assigned a value of:
			                   FS_ETPU_QOM_IMMEDIATE,
			                   FS_ETPU_QOM_USE_LAST_EVENT,
			                   FS_ETPU_QOM_USE_REF_ADDRESS
                   ref - This is the address of the reference when
                         first_match_mode==FS_ETPU_QOM_USE_REF_ADDRESS
                  loop - the number of iterations in loop mode
      event_array_size - the number of entries in the event array
           event_array - a structure containing the event table
RETURNS NOTES: Error code if channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : *This function does not configure the pin only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_qom_init (uint8_t channel, uint8_t priority, uint8_t mode,
                          uint8_t timebase, uint8_t init_pin,
                          uint8_t first_match_mode, uint32_t * ref,
                          uint8_t loop, uint8_t event_array_size,
                          union etpu_events_array *event_array);

/* TPU3 QOM API compatible functions to be added */

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
