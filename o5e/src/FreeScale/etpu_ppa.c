/**************************************************************************
 * FILE NAME: $RCSfile: etpu_ppa.c,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains ETPU Pulse or Period Accumulator function (PPA) API *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_ppa.c,v $
 * Revision 2.2  2009/02/09 13:51:30  r54529
 * Corrected comment of the init function timebase parameter options (FS_ETPU_PPA_INIT_TCR1/TCR2).
 *
 * Revision 2.1  2004/12/14 10:38:43  r47354
 * Set Accum and Period_count to 0 at initialisation.
 *
 * Revision 2.0  2004/12/09 15:49:12  r47354
 * no message
 *
 * Revision 1.1  2004/12/09 14:38:31  r47354
 * Updates as per QOM API rel_2_1
 *
 *........................................................................*
 * 0.1   G. Emerson  8/Sep/04  Initial version.                           *
 * 0.2   G. Emerson  12/Oct/04 Make hsr occur before channel is enabled.  *
 * 0.3   G. Emerson  27/Oct/04 Change return type of                      *
 *                             fs_etpu_ppa_get_immediate and              *
 *                             fs_etpu_ppa_get_accumulation               *
 *                             to uint8_t                                 *
 **************************************************************************/
#include "etpu_util.h"       /* Utility routines for working with the eTPU */
#include "etpu_ppa.h"        /* eTPU PPA API */
extern uint32_t fs_etpu_data_ram_start;

/***************************************************************************
FUNCTION     : fs_etpu_ppa_init
PURPOSE      : To initialize an eTPU channel to generate a ppa output.
INPUTS NOTES : This function has 5 parameters:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
              priority - This is the priority to assign to the channel.
                         This parameter should be assigned a value of:
                         ETPU_PRIORITY_HIGH, ETPU_PRIORITY_MIDDLE or
                         ETPU_PRIORITY_LOW.
              timebase - This is the timer to use as a reference for the ppa
                         function. This parameter should be assigned to a 
                         value of: FS_ETPU_PPA_INIT_TCR1 or 
                                   FS_ETPU_PPA_INIT_TCR2.
                  mode - This is the mode to use for PPA measurements. This 
                         parameter should be assigned a value of:
                         FS_ETPU_PPA_LOW_PULSE,
                         FS_ETPU_PPA_HIGH_PULSE,
                         FS_ETPU_PPA_FALLING_EDGE,
                         FS_ETPU_PPA_RISING_EDGE
             max_count - This is the number of periods or pulses that are 
                         accumulated before the measurement restarts. This
                         parameter should be assigned a value in the range
                         0 to 255. A value of zero or one results in the 
                         accumulation of one period or pulse width.
           sample_time - Defines the rate at which the measurement is updated 
                         in the absence of an edge.
RETURNS NOTES: Error code is channel could not be initialized. Error code that
                 can be returned are:
WARNING      : *This function does not configure the pin, only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/


int32_t fs_etpu_ppa_init( uint8_t channel, uint8_t priority, uint8_t timebase,
                       uint8_t mode, uint32_t max_count, uint32_t sample_time )

{
    uint32_t *pba;	/*parameter base address for channel*/
    uint8_t *pba8;	/*parameter base address for channel*/

/* disable channel before changing anything else */
fs_etpu_disable (channel);

/*only get parameter RAM if the channel has not been used before */

if (eTPU->CHAN[channel].CR.B.CPBA == 0 )
{
    /* get parameter RAM
    number of parameters passed from eTPU C code */
    pba = fs_etpu_malloc(FS_ETPU_PPA_NUM_PARMS);

	if (pba == 0)
	{
		return (FS_ETPU_ERROR_MALLOC);
	}
}
else /*set pba to what is in the CR register*/
{
	pba=fs_etpu_data_ram(channel);
}


pba8= (uint8_t *) pba;

/*write parameters*/

	*(pba + ((FS_ETPU_PPA_ACCUM_OFFSET - 1)>>2)) = 0;
	*(pba + ((FS_ETPU_PPA_PERIOD_COUNT_OFFSET - 1)>>2))  =  0;
	*(pba + ((FS_ETPU_PPA_SAMPLE_TIME_OFFSET - 1)>>2))  =  sample_time;
	*(pba + ((FS_ETPU_PPA_MAX_COUNT_OFFSET - 1)>>2))  =  max_count;

	*(pba8 + FS_ETPU_PPA_STAT_ACC_OFFSET )  =  0;

	eTPU->CHAN[channel].SCR.R = (mode);

/*write hsr*/
	eTPU->CHAN[channel].HSRR.R = timebase;
	
/*write chan config register */
    eTPU->CHAN[channel].CR.R =
        (priority << 28) + (FS_ETPU_PPA_TABLE_SELECT << 24) +
        (FS_ETPU_PPA_FUNCTION_NUMBER << 16) + (((uint32_t)pba -
        fs_etpu_data_ram_start) >> 3);

	return(0);
}

/******************************************************************************
FUNCTION     : fs_etpu_ppa_get_accumulation
PURPOSE      : This function call returns the status of the last measurement,
               and passes back the last accumulated pulse widths or accumulated 
               periods, in counts of the selected timebase used to initialize 
               the function: either TCR1 or TCR2
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
               *result - A pointer to where the PPA result will be stored.

RETURNS NOTES: returns status; if status is non zero the result has overflowed
WARNING      : *This function does not configure the pin, only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/

uint8_t fs_etpu_ppa_get_accumulation( uint8_t channel, uint32_t *result )
{
		uint32_t *pba;	/*parameter base address for channel*/
    uint8_t *pba8;
		uint8_t status;
		    
  	pba=fs_etpu_data_ram(channel);
  	pba8= (uint8_t *) pba;

    *result = *(pba + ((FS_ETPU_PPA_RESULT_OFFSET - 1)>>2)) & 0x00FFFFFF;
		 status = *(pba8 + FS_ETPU_PPA_STATUS_OFFSET );

		return (status);		
}

/******************************************************************************
FUNCTION     : fs_etpu_ppa_get_immediate
PURPOSE      : This function call returns the status of the last measurement,
               and passes back the last accumulated pulse widths or accumulated 
               periods, in counts of the selected timebase used to initialize 
               the function: either TCR1 or TCR2
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
               *result - A pointer to where the PPA result will be stored.
        *current_count - a pointer to where the current elapsed number 
                         of periods or pulse widths will be stored.
RETURNS NOTES: returns stat_acc; if stat_acc is non zero the result has overflowed
WARNING      : *This function does not configure the pin, only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/

uint8_t fs_etpu_ppa_get_immediate( uint8_t channel, uint32_t *result, uint32_t *current_count )
{
		uint32_t *pba;	/*parameter base address for channel*/
    uint8_t *pba8;
		uint8_t stat_acc;
		    
  	pba=fs_etpu_data_ram(channel);
  	pba8= (uint8_t *) pba;
  	
    *result = 	*(pba + ((FS_ETPU_PPA_ACCUM_OFFSET - 1)>>2)) & 0x00FFFFFF;
    *current_count= *(pba + ((FS_ETPU_PPA_PERIOD_COUNT_OFFSET - 1)>>2));
    stat_acc = *(pba8 + FS_ETPU_PPA_STAT_ACC_OFFSET );
    
		return (stat_acc);		

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