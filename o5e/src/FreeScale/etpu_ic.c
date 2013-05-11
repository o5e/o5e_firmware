/**************************************************************************
 * FILE NAME: $RCSfile: etpu_ic.c,v $        COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the eTPU Input Capture (IC) API.                    *
 *========================================================================*
 * ORIGINAL AUTHOR: Jeff Loeliger (r12110)                                *
 * $Log: etpu_ic.c,v $
 * Revision 2.0  2004/12/15 15:01:27  r12110
 * -Updated for 2.0 utils functions
 *
 * Revision 1.1  2004/12/15 14:55:56  r12110
 * -Initial version checked into CVS.
 *
 *........................................................................*
 * 0.1   J. Loeliger  31/Aug/04  Initial version.                         *
 * 0.2   J. Loeliger  19/Oct/04  Fixed comments where last and final were *
 *                                swapped.                                *
 **************************************************************************/
#include "etpu_util.h"		    /* Utility routines for working eTPU */
#include "etpu_ic.h"			/* eTPU IC API defines */
extern uint32_t fs_etpu_data_ram_start;

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif

/******************************************************************************
FUNCTION     : fs_etpu_ic_init
PURPOSE      : To initialize an eTPU channel to look for inputs.
INPUTS NOTES : This function has 6 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               priority - This is the priority to assign to the channel.
                          This parameter should be assigned a value of:
                          FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                          FS_ETPU_PRIORITY_LOW.
               mode - This should be assigned a value of: 
                        ETPU_IC_MODE_SINGLE or ETPU_IC_MODE_CONT.
               timebase - This is the timer to use as a reference for the 
                           PWM signal. This parameter should be assigned 
                           to a value of: ETPU_TCR1 or ETPU_TCR2.
               edge - This is the type of edge to detect and should be 
                        assigned a value of: ETPU_IC_FALLING_EDGE, 
                        ETPU_IC_RISING_EDGE or ETPU_IC_ANY_EDGE.
               max_count - This is the number of edges to count before
                            generating an exception request.
RETURNS NOTES: Error code if channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : *This function does not configure the pin only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_ic_init(uint8_t channel, uint8_t priority, uint8_t mode,
         uint8_t timebase, uint8_t edge, uint32_t max_count)
{
    uint32_t *pba;	/* parameter base address for 32&24 bit parameters */
    uint8_t *pba8;	/* parameter base address for 8 bit parameters */

	/* Disable channel to assign function safely */
	fs_etpu_disable( channel );

	pba = fs_etpu_malloc2( channel, ETPU_IC_NUM_PARMS);
	if (pba == 0) return (FS_ETPU_ERROR_MALLOC);

	pba8 = (uint8_t *)pba;

	/* write parameters to data memory */
	*(pba + ((ETPU_IC_MAXCOUNT_OFFSET - 1)>>2))  = max_count;
	*(pba8 + (ETPU_IC_FLAGS_OFFSET )) = edge;

	/* write FM (function mode) bits */
	eTPU->CHAN[channel].SCR.R = mode;

	/* write hsr to start channel running */
	if (timebase == FS_ETPU_TCR1 )
		eTPU->CHAN[channel].HSRR.R = ETPU_IC_INIT_TCR1;
	else
		eTPU->CHAN[channel].HSRR.R = ETPU_IC_INIT_TCR2;

	/* write channel configuration register and enable channel */
	eTPU->CHAN[channel].CR.R = (priority << 28) + (ETPU_IC_TABLE_SELECT << 24) +
	                        (ETPU_IC_FUNCTION_NUMBER << 16) +
	                        (((uint32_t)pba - fs_etpu_data_ram_start) >> 3);

	return(0);
}

/******************************************************************************
FUNCTION     : fs_etpu_ic_init_ram
PURPOSE      : To initialize an eTPU channel to look for inputs and use
                parameter RAM as the reference.
INPUTS NOTES : This function has 6 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               priority - This is the priority to assign to the channel.
                          This parameter should be assigned a value of:
                          FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                          FS_ETPU_PRIORITY_LOW.
               mode - This should be assigned a value of: 
                        ETPU_IC_MODE_SINGLE or ETPU_IC_MODE_CONT.
               edge - This is the type of edge to detect and should be 
                        assigned a value of: ETPU_IC_FALLING_EDGE, 
                        ETPU_IC_RISING_EDGE or ETPU_IC_ANY_EDGE.
               max_count - This is the number of edges to count before
                            generating an exception request.
               *address - Address of parameter RAM location to use as 
                           reference for edge.
RETURNS NOTES: Error code if channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : *This function does not configure the pin only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_ic_init_ram(uint8_t channel, uint8_t priority, uint8_t mode,
         uint8_t edge, uint32_t max_count, uint32_t *address)
{
    uint32_t *pba;	/* parameter base address for 32&24 bit parameters */
    uint8_t *pba8;	/* parameter base address for 8 bit parameters */

	/* Disable channel to assign function safely */
	fs_etpu_disable( channel );

	pba = fs_etpu_malloc2( channel, ETPU_IC_NUM_PARMS);
	if (pba == 0) return (FS_ETPU_ERROR_MALLOC);

	pba8 = (uint8_t *)pba;

	/* write parameters to data memory */
	*(pba + ((ETPU_IC_MAXCOUNT_OFFSET - 1)>>2))  = max_count;
	*(pba8 + (ETPU_IC_FLAGS_OFFSET )) = edge;
	*(pba + ((ETPU_IC_ADDRESS_OFFSET - 1)>>2))  = *address;

	/* write FM (function mode) bits */
	eTPU->CHAN[channel].SCR.R = mode;

	/* write hsr to start channel running */
	eTPU->CHAN[channel].HSRR.R = ETPU_IC_INIT_PRAM;

	/* write channel configuration register and enable channel */
	eTPU->CHAN[channel].CR.R = (priority << 28) + 
	                    (ETPU_IC_TABLE_SELECT << 24) +
	                    (ETPU_IC_FUNCTION_NUMBER << 16) +
	                    (((uint32_t)pba - fs_etpu_data_ram_start) >> 3);

	return(0);
}

/******************************************************************************
FUNCTION     : fs_etpu_ic_init_link
PURPOSE      : To initialize an eTPU channel to look for inputs and generate
                links when done.
INPUTS NOTES : This function has 8 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               priority - This is the priority to assign to the channel.
                          This parameter should be assigned a value of:
                          FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                          FS_ETPU_PRIORITY_LOW.
               mode - This should be assigned a value of: 
                        ETPU_IC_MODE_SINGLE or ETPU_IC_MODE_CONT.
               timebase - This is the timer to use as a reference for the 
                           PWM signal. This parameter should be assigned 
                           to a value of: ETPU_TCR1 or ETPU_TCR2.
               edge - This is the type of edge to detect and should be 
                        assigned a value of: ETPU_IC_FALLING_EDGE, 
                        ETPU_IC_RISING_EDGE or ETPU_IC_ANY_EDGE.
               max_count - This is the number of edges to count before
                            generating an exception request.
               link1 - This is a packed 32 bit parameter with 4 8 bit 
                        channel numbers in it.
               link2 - This is a packed 32 bit parameter with 4 8 bit 
                        channel numbers in it.
RETURNS NOTES: Error code if channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : *This function does not configure the pin only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_ic_init_link(uint8_t channel, uint8_t priority, uint8_t mode,
         uint8_t timebase, uint8_t edge, uint32_t max_count, uint32_t link1,
         uint32_t link2)
{
    uint32_t *pba;	/* parameter base address for 32&24 bit parameters */
    uint8_t *pba8;	/* parameter base address for 8 bit parameters */

	/* Disable channel to assign function safely */
	fs_etpu_disable( channel );

	pba = fs_etpu_malloc2( channel, ETPU_IC_NUM_PARMS);
	if (pba == 0) return (FS_ETPU_ERROR_MALLOC);

	pba8 = (uint8_t *)pba;

	/* write parameters to data memory */
	*(pba + ((ETPU_IC_MAXCOUNT_OFFSET - 1)>>2))  = max_count;
	*(pba8 + (ETPU_IC_FLAGS_OFFSET )) = edge;
	*(pba + ((ETPU_IC_LINK1_OFFSET)>>2))  = link1;
	*(pba + ((ETPU_IC_LINK2_OFFSET)>>2))  = link2;

	/* write FM (function mode) bits */
	eTPU->CHAN[channel].SCR.R = mode + ETPU_IC_MODE_LINK;

	/* write hsr to start channel running */
	if (timebase == FS_ETPU_TCR1 )
		eTPU->CHAN[channel].HSRR.R = ETPU_IC_INIT_TCR1;
	else
		eTPU->CHAN[channel].HSRR.R = ETPU_IC_INIT_TCR2;

	/* write channel configuration register and enable channel */
	eTPU->CHAN[channel].CR.R = (priority << 28) + (ETPU_IC_TABLE_SELECT << 24) +
	                        (ETPU_IC_FUNCTION_NUMBER << 16) +
	                        (((uint32_t)pba - fs_etpu_data_ram_start) >> 3);

	return(0);
}

/******************************************************************************
FUNCTION     : fs_etpu_ic_init_link_ram
PURPOSE      : To initialize an eTPU channel to look for inputs using a
                parameter RAM refernce and generating links when done.
INPUTS NOTES : This function has 8 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               priority - This is the priority to assign to the channel.
                          This parameter should be assigned a value of:
                          FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                          FS_ETPU_PRIORITY_LOW.
               mode - This should be assigned a value of: 
                        ETPU_IC_MODE_SINGLE or ETPU_IC_MODE_CONT.
               edge - This is the type of edge to detect and should be 
                        assigned a value of: ETPU_IC_FALLING_EDGE, 
                        ETPU_IC_RISING_EDGE or ETPU_IC_ANY_EDGE.
               max_count - This is the number of edges to count before
                            generating an exception request.
               *address - Address of parameter RAM location to use as 
                           reference for edge.
               link1 - This is a packed 32 bit parameter with 4 8 bit 
                        channel numbers in it.
               link2 - This is a packed 32 bit parameter with 4 8 bit 
                        channel numbers in it.
RETURNS NOTES: Error code if channel could not be initialized. Error code that
                 can be returned is: FS_ETPU_ERROR_MALLOC
WARNING      : *This function does not configure the pin only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_ic_init_link_ram(uint8_t channel, uint8_t priority, uint8_t mode,
         uint8_t edge, uint32_t max_count, uint32_t link1, uint32_t link2,
         uint32_t *address)
{
    uint32_t *pba;	/* parameter base address for 32&24 bit parameters */
    uint8_t *pba8;	/* parameter base address for 8 bit parameters */

	/* Disable channel to assign function safely */
	fs_etpu_disable( channel );

	pba = fs_etpu_malloc2( channel, ETPU_IC_NUM_PARMS);
	if (pba == 0) return (FS_ETPU_ERROR_MALLOC);

	pba8 = (uint8_t *)pba;

	/* write parameters to data memory */
	*(pba + ((ETPU_IC_MAXCOUNT_OFFSET - 1)>>2))  = max_count;
	*(pba8 + (ETPU_IC_FLAGS_OFFSET )) = edge;
	*(pba + ((ETPU_IC_ADDRESS_OFFSET - 1)>>2))  = *address;
	*(pba + ((ETPU_IC_LINK1_OFFSET - 1)>>2))  = link1;
	*(pba + ((ETPU_IC_LINK2_OFFSET - 1)>>2))  = link2;

	/* write FM (function mode) bits */
	eTPU->CHAN[channel].SCR.R = mode + ETPU_IC_MODE_LINK;

	/* write hsr to start channel running */
	eTPU->CHAN[channel].HSRR.R = ETPU_IC_INIT_PRAM;

	/* write channel configuration register and enable channel */
	eTPU->CHAN[channel].CR.R = (priority << 28) + 
	               (ETPU_IC_TABLE_SELECT << 24) +
	               (ETPU_IC_FUNCTION_NUMBER << 16) +
	               (((uint32_t)pba - fs_etpu_data_ram_start) >> 3);

	return(0);
}

/* Data read functions */

/******************************************************************************
FUNCTION     : fs_etpu_ic_read_trans_count
PURPOSE      : To read the current number of transition counted.
INPUTS NOTES : This function has 1 parameter:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
RETURNS NOTES: The current number of transitions counted.
******************************************************************************/
int32_t fs_etpu_ic_read_trans_count(uint8_t channel)
{
    return( fs_etpu_get_chan_local_24s(channel, ETPU_IC_TRANSCOUNT_OFFSET));
}

/******************************************************************************
FUNCTION     : fs_etpu_ic_read_final_time
PURPOSE      : To read the final tranistion refernce value.
INPUTS NOTES : This function has 1 parameter:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
RETURNS NOTES: The reference value of the final detected transition.
******************************************************************************/
int32_t fs_etpu_ic_read_final_time(uint8_t channel)
{
    return( fs_etpu_get_chan_local_24s(channel, ETPU_IC_FINAL_OFFSET));
}

/******************************************************************************
FUNCTION     : fs_etpu_ic_read_last_time
PURPOSE      : To read the last tranistion refernce value.
INPUTS NOTES : This function has 1 parameter:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
RETURNS NOTES: The reference value of the last detected transition.
******************************************************************************/
int32_t fs_etpu_ic_read_last_time(uint8_t channel)
{
    return( fs_etpu_get_chan_local_24s(channel, ETPU_IC_LAST_OFFSET));
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
#if __CWCC__
#pragma pop
#endif