/**************************************************************************
 * FILE NAME: $RCSfile: etpu_qom.c,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains  ETPU Queued Output Match (QOM) API                 *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_qom.c,v $
 * Revision 2.1  2004/12/20 08:47:52  r47354
 * Fix CW compiler warnings.
 *
 * Revision 2.0  2004/12/01 11:29:50  r47354
 * Changes to remove etpu_config.h from builds.
 *
 * Revision 1.3  2004/11/22 11:38:16  r47354
 * Make hsr occur before channel is enabled. Write FM bits before enabling channel.
 *
 * Revision 1.2  2004/11/11 14:25:38  r47354
 * Tidy up and formatting.
 *
 * Revision 1.1  2004/11/09 11:25:44  r47354
 * No changes to code, just updated for CVS and checked into CVS.
 *........................................................................*
 * 0.1   G. Emerson  2/Apr/04  Initial version.                           *
 * 0.2   G. Emerson  28/Apr/04 Remove fs_etpu_qom_send_link               *
 * 0.3   G. Emerson  20/May/04 Add details on *INIT*LINK                  *
 * 0.4   G. Emerson  27/May/04 Post review tidy up                        *
 * 0.5   G. Emerson  21/Jun/04 Post review tidy up                        *
 * 0.6                         Update for new build structure.            *
 * 0.7   G. Emerson            Minor change for GCT compatibility.        *
 * 0.8   G.Emerson  25/June/04 Add FS_ prefix to following macros         *
 *                             ETPU_QOM_TABLE_START_PTR_OFFSET            *
 *                             ETPU_QOM_TABLE_END_PTR_OFFSET              *
 *                             ETPU_QOM_LOOP_COUNT_OFFSET                 *
 *                             ETPU_QOM_OPTION_OFFSET                     *
 *                             ETPU_QOM_REF_ADDR_PTR_OFFSET               *
 *                             ETPU_QOM_FUNCTION_NUMBER                   *
 *                             ETPU_QOM_TABLE_SELECT                      *
 *                             ETPU_QOM_NUM_PARMS                         *
 * 0.9   G.Emerson  16/July/04 Add Disclaimer & update for new header file*
 * 0.10  G. Emerson  1/Nov/04  Remove path in #include                    *
 *                             Fix typo in rev# (0.8 should have been 0.9)*   
 **************************************************************************/
#include "etpu_util.h"       /* Utility routines for working with the eTPU */
#include "etpu_qom.h"        /* eTPU QOM API */
extern uint32_t fs_etpu_data_ram_start;
/******************************************************************************
FUNCTION     : fs_etpu_qom_init
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
WARNING      : *This function does not configure the pin, only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/

int32_t fs_etpu_qom_init (uint8_t channel, uint8_t priority, uint8_t mode,
                          uint8_t timebase, uint8_t init_pin,
                          uint8_t first_match_mode, uint32_t * ref, uint8_t Loop,
                          uint8_t event_array_size,
                          union etpu_events_array *event_array)
{
    uint32_t *pba;              /*parameter base address for channel */
    uint32_t *pba_array;        /*parameter base address for match table */
    uint32_t *Table_Start_Ptr;
    uint32_t *Table_End_Ptr;
    uint16_t event_array_size_rounded;
    uint16_t x, x2;
    uint8_t Option;
    uint8_t channel_reuse = 0;
    uint8_t *pba8;

/* disable channel before changing anything else */
    fs_etpu_disable (channel);

/*generate option */
    Option = 0;                 /*clear out any erroneous data */
    Option = (uint8_t)(timebase << 7 | first_match_mode);

/*only get parameter RAM if the channel has not been used before */

    if (eTPU->CHAN[channel].CR.B.CPBA == 0)
      {
          /* get parameter RAM
             number of parameters passed from eTPU C code */
          pba = fs_etpu_malloc (FS_ETPU_QOM_NUM_PARMS);

          if (pba == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else                        /*set pba to what is in the CR register */
      {
          channel_reuse = 1;
          pba = fs_etpu_data_ram (channel);
      }
/* Set the table start pointer variable so that the pointer is relative to the
  eTPU address map .The queue data begins after the parameter data. */
    Table_Start_Ptr =
        pba + (FS_ETPU_QOM_NUM_PARMS) / 4 -
        (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4));
    Table_End_Ptr = Table_Start_Ptr + (event_array_size - 1);

    pba8 = (uint8_t *) pba;

/*write parameters*/
    if (first_match_mode == FS_ETPU_QOM_USE_REF_ADDRESS)
      {
/*only write reference pointer if using that mode. */

          *(pba + ((FS_ETPU_QOM_REF_ADDR_PTR_OFFSET - 1) >> 2)) =
              (uint24_t) ref;
      }

/*only write new Table pointer if not running in last match time mode. */
    *(pba + ((FS_ETPU_QOM_TABLE_START_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_Start_Ptr);
    *(pba + ((FS_ETPU_QOM_TABLE_END_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_End_Ptr);

    *(pba8 + FS_ETPU_QOM_LOOP_COUNT_OFFSET) = (uint8_t) Loop;
    *(pba8 + FS_ETPU_QOM_OPTION_OFFSET) = (uint8_t) Option;

    if (channel_reuse == 0)
      {
          /* get parameter RAM
             for the Event Table

             following code ensures that the pba ends up on a times 8 
             boundary; so if the user requests an odd event_array_size there
             will be 1 wasted 32bit PRAM location. */

          x2 = (uint16_t)(event_array_size + 1);
          x2 = (uint16_t)(x2 >> 1);
          event_array_size_rounded = (uint16_t)(x2 << 1);
          pba_array = fs_etpu_malloc ((uint16_t)(event_array_size_rounded << 2));

          if (pba_array == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else                        /* re-using parameter RAM which has already been allocated */
      {
          pba_array = (pba + (FS_ETPU_QOM_NUM_PARMS) / 4);
      }

/*write event table*/
    for (x = 0; x < event_array_size; x++)
      {
          *(pba_array + x) = event_array[x].p;
      }

/*write hsr*/
    eTPU->CHAN[channel].HSRR.R = init_pin;

    eTPU->CHAN[channel].SCR.R = (mode);

/*write chan config register */
    eTPU->CHAN[channel].CR.R =
        (priority << 28) + (FS_ETPU_QOM_TABLE_SELECT << 24) +
        (FS_ETPU_QOM_FUNCTION_NUMBER << 16) + (((uint32_t)pba -
        fs_etpu_data_ram_start) >> 3);

    return (0);
}

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
