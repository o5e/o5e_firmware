/**************************************************************************
 * FILE NAME: $RCSfile: etpu_oc.c,v $        COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains  ETPU  Output Compare (OC) API                      *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_oc.c,v $
 * Revision 2.0  2004/12/07 14:03:08  r47354
 * no message
 *
 * Revision 1.3  2004/12/07 14:01:07  r47354
 * Formatting tidy up
 *
 * Revision 1.2  2004/12/07 13:12:35  r47354
 * Updates as per QOM API rel_2_1
 *
 *........................................................................*
 * 0.1   G. Emerson   21/May/04     Initial version.                      *
 * 0.2   G. Emerson   21/May/04     Disable channel to begin with         *
 * 0.3                              Updated for new build structure.      *
 * 0.4   G.Emerson  25/June/04 Add FS_ prefix to following macros         *
 *                             ETPU_QOM_TABLE_START_PTR_OFFSET            *
 *                             ETPU_QOM_TABLE_END_PTR_OFFSET              *
 *                             ETPU_QOM_LOOP_COUNT_OFFSET                 *
 *                             ETPU_QOM_OPTION_OFFSET                     *
 *                             ETPU_QOM_REF_ADDR_PTR_OFFSET               *
 *                             ETPU_QOM_FUNCTION_NUMBER                   *
 *                             ETPU_QOM_TABLE_SELECT                      *
 *                             ETPU_QOM_NUM_PARMS                         *
 *                             ETPU_QOM_LAST_MATCH_TIME_OFFSET            *
 * 0.5   G.Emerson  16/July/04 Add Disclaimer                             *
 **************************************************************************/
#include "etpu_util.h"          /* Utility routines for working with the eTPU */
#include "etpu_oc.h"            /* eTPU QOM API */
extern uint32_t fs_etpu_data_ram_start;
/**************************************************************************
FUNCTION     : fs_etpu_oc_init_immed
PURPOSE      : To initialize an eTPU channel to generate an immediate OC output.
INPUTS NOTES : This function has 5 parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
              priority - This is the priority to assign to the channel.
                         This parameter should be assigned a value of:
                         ETPU_PRIORITY_HIGH, ETPU_PRIORITY_MIDDLE or
                         ETPU_PRIORITY_LOW.
              init_pin - The  state  the pin is driven to at initialisation.
               		       This parameter should be assigned a value of:
               		        FS_ETPU_OC_INIT_PIN_LOW
               		        FS_ETPU_OC_INIT_PIN_HIGH
               		        FS_ETPU_OC_INIT_PIN_NO_CHANGE
            Offset1/2 - these are structures whose members are :
            		        timebase: the timebase for match and capture.
            		                  Valid values are:
            		                  FS_ETPU_OC_MATCH_TCR1_CAPTURE_TCR1
            		                  FS_ETPU_OC_MATCH_TCR2_CAPTURE_TCR2
            		        offset:   The number of selected TCR counts from now
            		                  to the event
	                      pin:      The future pin state; valid values are
	                                FS_ETPU_OC_PIN_HIGH
	                                FS_ETPU_OC_PIN_LOW
******************************************************************************
RETURNS NOTES: Error code is channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : This function does not configure the pin, only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/

uint8_t fs_etpu_oc_init_immed (uint8_t channel, uint8_t priority,
                               struct offset Offset1, struct offset Offset2,
                               uint8_t init_pin)
{
    uint32_t *pba;              /*parameter base address for channel */
    uint32_t *pba_array;        /*parameter base address for match table */
    uint32_t *Table_Start_Ptr;
    uint32_t *Table_End_Ptr;
    uint32_t array_size_rounded;
    uint8_t Option;
    uint8_t *pba8;
    uint8_t channel_reuse = 0;
    uint8_t mode;

/* disable channel before changing anything else */
    fs_etpu_disable (channel);

/*generate option*/
    Option = 0;                 /*clear out any erroneous data */
    Option = Offset1.timebase << 7 | FS_ETPU_QOM_IMMEDIATE;

    mode = FS_ETPU_QOM_SINGLE_SHOT;

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
/* Set the table start pointer variable so that the pointer is relative to 
   the  eTPU address map. The queue data begins after the parameter data. */

    Table_Start_Ptr =
        pba + (FS_ETPU_QOM_NUM_PARMS) / 4 -
        (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4));
    Table_End_Ptr = Table_Start_Ptr + 1;

    pba8 = (uint8_t *) pba;

/*write parameters*/

    *(pba + ((FS_ETPU_QOM_TABLE_START_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_Start_Ptr);
    *(pba + ((FS_ETPU_QOM_TABLE_END_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_End_Ptr);
    *(pba8 + FS_ETPU_QOM_OPTION_OFFSET) = (uint8_t) Option;

    if (channel_reuse == 0)
      {
          /* get parameter RAM
             for the Event Table */

          pba_array = fs_etpu_malloc (8);

          if (pba_array == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else  /* re-using parameter RAM which has already been allocated */
      {
          pba_array = (pba + (FS_ETPU_QOM_NUM_PARMS) / 4);
      }

/*write event table*/
    *(pba_array) = (Offset1.offset << 1) + Offset1.pin;
    *(pba_array + 1) = (Offset2.offset << 1) + Offset2.pin;

/*write hsr*/
    eTPU->CHAN[channel].HSRR.R = init_pin;

    eTPU->CHAN[channel].SCR.R = (mode);

/*write chan config register */
    eTPU->CHAN[channel].CR.R =
        (priority << 28) + (FS_ETPU_QOM_TABLE_SELECT << 24) +
        (FS_ETPU_QOM_FUNCTION_NUMBER << 16) + (((uint32_t) pba -
        fs_etpu_data_ram_start) >> 3);

    return (0);
};

/******************************************************************************
/******************************************************************************
/*FUNCTION     : fs_etpu_oc_init_value
/*PURPOSE      : To initialize an eTPU channel to generate an OC output 
                 in value mode
INPUTS NOTES : This function has 5 parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
              priority - This is the priority to assign to the channel.
                         This parameter should be assigned a value of:
                         ETPU_PRIORITY_HIGH, ETPU_PRIORITY_MIDDLE or
                         ETPU_PRIORITY_LOW.
              init_pin - The  state  the pin is driven to at initialisation.
               		       This parameter should be assigned a value of:
               		        FS_ETPU_OC_INIT_PIN_LOW
               		        FS_ETPU_OC_INIT_PIN_HIGH
               		        FS_ETPU_OC_INIT_PIN_NO_CHANGE
             Offset1/2 - these are structures whose members are :
            		         timebase: the timebase for match and capture. 
            		         Valid values are:
            		                  FS_ETPU_OC_MATCH_TCR1_CAPTURE_TCR1
            		                  FS_ETPU_OC_MATCH_TCR2_CAPTURE_TCR2
            		        offset:   The number of selected TCR counts from 
            		                  now to the event
	                      pin:      The future pin state; valid values are
	                                FS_ETPU_OC_PIN_HIGH
	                                FS_ETPU_OC_PIN_LOW
	                      In this case Offset1.offset is irrelevant
*******************************************************************************
RETURNS NOTES: Error code is channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : This function does not configure the pin, only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
*******************************************************************************/

uint8_t fs_etpu_oc_init_value (uint8_t channel, uint8_t priority,
                               struct offset Offset1, struct offset Offset2,
                               uint8_t init_pin)
{
    uint32_t *pba;              /*parameter base address for channel */
    uint32_t *pba_array;        /*parameter base address for match table */
    uint32_t *Table_Start_Ptr;
    uint32_t *Table_End_Ptr;
    uint32_t array_size_rounded;
    uint8_t Option;
    uint8_t *pba8;
    uint8_t channel_reuse = 0;
    uint8_t mode;

/* disable channel before changing anything else */
    fs_etpu_disable (channel);

/*generate option*/
    Option = 0;                 /*clear out any erroneous data */
    Option = Offset1.timebase << 7 | FS_ETPU_QOM_USE_LAST_EVENT;

    mode = FS_ETPU_QOM_SINGLE_SHOT;

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
/* Set the table start pointer variable so that the pointer is relative 
   to the  eTPU address map. The queue data begins after the parameter data.*/

    Table_Start_Ptr =
        pba + (FS_ETPU_QOM_NUM_PARMS) / 4 -
        (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4));
    Table_End_Ptr = Table_Start_Ptr + 1;

    pba8 = (uint8_t *) pba;

/*write parameters*/
    *(pba + ((FS_ETPU_QOM_TABLE_START_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_Start_Ptr);
    *(pba + ((FS_ETPU_QOM_TABLE_END_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_End_Ptr);
    *(pba + ((FS_ETPU_QOM_LAST_MATCH_TIME_OFFSET - 1) >> 2)) =
        (uint24_t) (Offset1.offset);
    *(pba8 + FS_ETPU_QOM_OPTION_OFFSET) = (uint8_t) Option;

    if (channel_reuse == 0)
      {
          /* get parameter RAM
             for the Event Table */

          pba_array = fs_etpu_malloc (8);

          if (pba_array == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else /* re-using parameter RAM which has already been allocated */
      {
          pba_array = (pba + (FS_ETPU_QOM_NUM_PARMS) / 4);
      }

/*write hsr*/
    eTPU->CHAN[channel].HSRR.R = init_pin;

    eTPU->CHAN[channel].SCR.R = (mode);

/*write chan config register */
    eTPU->CHAN[channel].CR.R =
        (priority << 28) + (FS_ETPU_QOM_TABLE_SELECT << 24) +
        (FS_ETPU_QOM_FUNCTION_NUMBER << 16) + (((uint32_t) pba -
         fs_etpu_data_ram_start) >> 3);

    return (0);
};

/******************************************************************************
FUNCTION     : fs_etpu_oc_init_ref
PURPOSE      : To initialize an eTPU channel to generate a OC output with 
               respect to a reference value
INPUTS NOTES : This function has 6 parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
              priority - This is the priority to assign to the channel.
                         This parameter should be assigned a value of:
                         ETPU_PRIORITY_HIGH, ETPU_PRIORITY_MIDDLE or
                         ETPU_PRIORITY_LOW.
              init_pin - The  state  the pin is driven to at initialisation.
               		       This parameter should be assigned a value of:
               		        FS_ETPU_OC_INIT_PIN_LOW
               		        FS_ETPU_OC_INIT_PIN_HIGH
               		        FS_ETPU_OC_INIT_PIN_NO_CHANGE
             Offset1/2 - these are structures whose members are :
            		         timebase: the timebase for match and capture.
            		                  Valid values are:
            		                  FS_ETPU_OC_MATCH_TCR1_CAPTURE_TCR1
            		                  FS_ETPU_OC_MATCH_TCR2_CAPTURE_TCR2
            		        offset:   The number of selected TCR counts from the 
            		                  reference to the event
	                      pin:      The future pin state; valid values are
	                                FS_ETPU_OC_PIN_HIGH
	                                FS_ETPU_OC_PIN_LOW
	                      In this case Offset1.offset is irrelevant
                  ref - This is the address of the reference
*******************************************************************************
RETURNS NOTES: Error code is channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : This function does not configure the pin, only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/
uint8_t fs_etpu_oc_init_ref (uint8_t channel, uint8_t priority,
                             struct offset Offset1, struct offset Offset2,
                             uint8_t init_pin, uint32_t * ref)
{
    uint32_t *pba;              /*parameter base address for channel */
    uint32_t *pba_array;        /*parameter base address for match table */
    uint32_t *Table_Start_Ptr;
    uint32_t *Table_End_Ptr;
    uint32_t array_size_rounded;
    uint8_t Option;
    uint8_t *pba8;
    uint8_t channel_reuse = 0;
    uint8_t mode;

/* disable channel before changing anything else */
    fs_etpu_disable (channel);

/*generate option*/
    Option = 0;                 /*clear out any erroneous data */
    Option = Offset1.timebase << 7 | FS_ETPU_QOM_USE_REF_ADDRESS;

    mode = FS_ETPU_QOM_SINGLE_SHOT;

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
/* Set the table start pointer variable so that the pointer is relative to
   the  eTPU address map The queue data begins after the parameter data. */

    Table_Start_Ptr =
        pba + (FS_ETPU_QOM_NUM_PARMS) / 4 -
        (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4));
    Table_End_Ptr = Table_Start_Ptr + 1;

    pba8 = (uint8_t *) pba;

/*write parameters*/

    *(pba + ((FS_ETPU_QOM_REF_ADDR_PTR_OFFSET - 1) >> 2)) = (uint24_t) ref;
    *(pba + ((FS_ETPU_QOM_TABLE_START_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_Start_Ptr);
    *(pba + ((FS_ETPU_QOM_TABLE_END_PTR_OFFSET - 1) >> 2)) =
        (uint24_t) (Table_End_Ptr);
    *(pba8 + FS_ETPU_QOM_OPTION_OFFSET) = (uint8_t) Option;

    if (channel_reuse == 0)
      {
          /* get parameter RAM
             for the Event Table */

          pba_array = fs_etpu_malloc (8);

          if (pba_array == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else /* re-using parameter RAM which has already been allocated */
      {
          pba_array = (pba + (FS_ETPU_QOM_NUM_PARMS) / 4);
      }

/*write event table*/
    *(pba_array) = (Offset1.offset << 1) + Offset1.pin;
    *(pba_array + 1) = (Offset2.offset << 1) + Offset2.pin;

/*write hsr*/
    eTPU->CHAN[channel].HSRR.R = init_pin;

    eTPU->CHAN[channel].SCR.R = (mode);

/*write chan config register */
    eTPU->CHAN[channel].CR.R =
        (priority << 28) + (FS_ETPU_QOM_TABLE_SELECT << 24) +
        (FS_ETPU_QOM_FUNCTION_NUMBER << 16) + (((uint32_t) pba -
         fs_etpu_data_ram_start) >> 3);

    return (0);
};

/******************************************************************************
FUNCTION     : fs_etpu_oc_data
PURPOSE      : To get the last match time for the 2nd event on an OC channel
INPUTS NOTES : This function has 6 parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
******************************************************************************/
int32_t
fs_etpu_oc_data (uint8_t channel)
{
    int32_t last_match_time2;

    last_match_time2 =
        fs_etpu_get_chan_local_24s (channel,
                                    FS_ETPU_QOM_LAST_MATCH_TIME_OFFSET);

    return (last_match_time2);
};

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
