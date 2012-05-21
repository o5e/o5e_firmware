/**************************************************************************
 * FILE NAME: $RCSfile: etpu_pwm.c,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the eTPU Pulse Width Modulation(PWM) API.           *
 *========================================================================*
 * ORIGINAL AUTHOR: Jeff Loeliger (r12110)                                *
 * $Log: etpu_pwm.c,v $
 * Revision 2.0  2004/12/22 13:46:31  r12110
 * -First release into CVS.
 * -Added support for utils 2.0 functions.
 *
 *........................................................................*
 * 0.1   J. Loeliger  16/Sep/03  Initial version.                         *
 * 0.8   J. Loeliger  19/Jul/04   Fixed pwm_update need to use coherent   *
 *                                 update HSR.                            *
 * 0.9   J. Loeliger  20/Jul/04   Updated for new eTPU code, added masks  *
 *                                 for acceses to period parameter.       *
 **************************************************************************/
#include "etpu_util.h"		    /* Utility routines for working eTPU */
#include "etpu_pwm.h"			/* eTPU PWM API defines */
extern uint32_t fs_etpu_data_ram_start;

/******************************************************************************
FUNCTION     : fs_etpu_pwm_init
PURPOSE      : To initialize an eTPU channel to generate a PWM output.
INPUTS NOTES : This function has 7 parameters:
               channel - This is the channel number.
                          0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               freq - This is the frequency of the PWM. This is an unint32_t
                        but the value range is only 24 bits. The range of
                        this parameter is determine by the complete system but
                        normally would be between 1Hz-100kHz.
               duty - This is the initial duty cycle of the PWM. This is a
                        uint16_t with a range of 0-10000. To represent 0-100%
                        with 0.01% resolution.
               priority - This is the priority to assign to the channel.
                          This parameter should be assigned a value of:
                          FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                          FS_ETPU_PRIORITY_LOW.
               polarity - This is the polarity of the channel. This parameter
                            should be assigned a value of:
                            FS_ETPU_PWM_ACTIVEHIGH or FS_ETPU_PWM_ACTIVELOW
               timebase - This is the timer to use as a reference for the PWM
                         signal. This parameter should be assigned to a value
                         of: FS_ETPU_TCR1 or FS_ETPU_TCR2.
               timebase_freq - This is the frequency of the selected timebase.
                 The range of this is the same as the range of the timebase
                 frequency on the device. This parameter is a uint32_t.
RETURNS NOTES: Error code if channel could not be initialized. Error code that
                 can be returned are: FS_ETPU_ERROR_MALLOC ,
                 FS_ETPU_ERROR_FREQ
WARNING      : *This function does not configure the pin only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_pwm_init( uint8_t channel, uint8_t priority, uint32_t freq, 
      uint16_t duty, uint8_t polarity, uint8_t timebase, uint32_t timebase_freq)
{
    uint32_t *pba;	/* parameter base address for channel */
    uint32_t chan_period;

	/* Disable channel to assign function safely */
	fs_etpu_disable( channel );

    if (eTPU->CHAN[channel].CR.B.CPBA == 0 )
    {
        /* get parameter RAM
        number of parameters passed from eTPU C code */
        pba = fs_etpu_malloc(FS_ETPU_PWM_NUM_PARMS);

    	if (pba == 0)
    	{
    		return (FS_ETPU_ERROR_MALLOC);
    	}
    }
    else /*set pba to what is in the CR register*/
    {
    	pba=fs_etpu_data_ram(channel);
    }

    /* Determine frequency of output waveform */
	chan_period = timebase_freq / freq;

	if ((chan_period == 0) || (chan_period > 0x007FFFFF ))
		return( FS_ETPU_ERROR_FREQ);

	/* write parameters to data memory */
	*(pba + ((FS_ETPU_PWM_PERIOD_OFFSET - 1)>>2)) = chan_period;
	*(pba + ((FS_ETPU_PWM_ACTIVE_OFFSET - 1)>>2))  = 
			(chan_period * duty) / 10000;

	/* write channel configuration register */
	eTPU->CHAN[channel].CR.R = (priority << 28) + 
				(FS_ETPU_PWM_TABLE_SELECT << 24) +
	                        (FS_ETPU_PWM_FUNCTION_NUMBER << 16) +
	                        (((uint32_t)pba - fs_etpu_data_ram_start)>>3);

	/* write FM (function mode) bits */
	eTPU->CHAN[channel].SCR.R = (timebase << 1) + polarity;

	/* write hsr to start channel running */
	eTPU->CHAN[channel].HSRR.R = FS_ETPU_PWM_INIT;

	return(0);
}

/******************************************************************************
FUNCTION     : fs_etpu_pwm_duty
PURPOSE      : To update a channel dutycycle using a 16 bit integer value. The
                 ineteger value is the percentage *100, so 20% would be 2000.
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               duty - This is the duty cycle of the PWM. This is a
                        uint16 with a range of 0-10000. To represent 0-100%
                        with 0.01% resolution.
RETURNS NOTES: none
WARNING      :
******************************************************************************/
void fs_etpu_pwm_duty( uint8_t channel, uint16_t duty)
{
	uint32_t *pba;
	uint32_t period;

	pba = fs_etpu_data_ram (channel);

	period = *(pba + FS_ETPU_PWM_PERIOD_OFFSET-1);
	period = period & 0xFFFFFF;

	pba += ((FS_ETPU_PWM_ACTIVE_OFFSET-1)>>2);
	*pba = (period * duty) / 10000;

}

/******************************************************************************
FUNCTION     : fs_etpu_pwm_duty_immed
PURPOSE      : To immedately update a channel dutycycle using a 16 bit integer value.
                This function will update the dutycycle during the current period
                if possible. The ineteger value is the percentage *100, so 20% would
                be 2000.
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               duty - This is the duty cycle of the PWM. This is a
                        uint16 with a range of 0-10000. To represent 0-100%
                        with 0.01% resolution.
******************************************************************************/
void fs_etpu_pwm_duty_immed( uint8_t channel, uint16_t duty)
{
	uint32_t *pba;
	uint32_t period;

	pba = fs_etpu_data_ram (channel);

	period = *(pba + FS_ETPU_PWM_PERIOD_OFFSET-1);

	pba += ((FS_ETPU_PWM_ACTIVE_OFFSET-1)>>2);
	period = period & 0xFFFFFF;
	*pba = (period * duty) / 10000;

	/* do immediate update of duty cycle is possible */
	eTPU->CHAN[channel].HSRR.R = FS_ETPU_PWM_IMM_UPDATE;

}

/******************************************************************************
FUNCTION     : fs_etpu_pwm_update
PURPOSE      : To update a PWM output's frequency and dutycycle
INPUTS NOTES : This function has 6 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               freq - This is the frequency of the PWM. This is an unint32_t
                        but the value range is only 24 bits. The range of
                        this parameter is determine by the complete system but
                        normally would be between 1Hz-100kHz.
               duty - This is the initial duty cycle of the PWM. This is a
                        uint16_t with a range of 0-10000. To represent 0-100%
                        with 0.01% resolution.
               timebase_freq - This is the frequency of the selected timebase.
                 The range of this is the same as the range of the timebase
                 frequency on the device. This parameter is a uint32_t.
RETURNS NOTES: Error code if frequency is out of range: FS_ETPU_ERROR_FREQ
******************************************************************************/
int32_t fs_etpu_pwm_update( uint8_t channel, uint32_t freq, uint16_t duty, 
       uint32_t timebase_freq)
{
	uint32_t *pba;
	uint32_t chan_period;

	pba = fs_etpu_data_ram (channel);

    /* Determine frequency of output waveform */
	chan_period = timebase_freq / freq;

	if ((chan_period == 0) || (chan_period > 0x007FFFFF ))
		return( FS_ETPU_ERROR_FREQ);

	/* write parameters to data memory */
	*(pba + ((FS_ETPU_PWM_CO_PERIOD_OFFSET - 1)>>2)) = chan_period;
	*(pba + ((FS_ETPU_PWM_CO_ACTIVE_OFFSET - 1)>>2))  = (chan_period * duty) / 10000;

	eTPU->CHAN[channel].HSRR.R = FS_ETPU_PWM_CO_UPDATE;
	
	return 0;

}

/******************************************************************************
FUNCTION     : fs_etpu_pwm_get_freq
PURPOSE      : To determine the actual frequency by the PWM channel.
INPUTS NOTES : This function has 1 parameters:
               channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
               timebase_freq - This is the frequency of the selected timebase.
                 The range of this is the same as the range of the timebase
                 frequency on the device. This parameter is a uint32_t.
RETURNS NOTES: The actual frequency as an integer.
WARNING      :
******************************************************************************/
uint32_t fs_etpu_pwm_get_freq( uint8_t channel, uint32_t timebase_freq)
{
	uint32_t chan_period;
	uint32_t *pba;

	pba = fs_etpu_data_ram (channel);

	chan_period = *(pba + ((FS_ETPU_PWM_PERIOD_OFFSET - 1)>>2));
	chan_period = chan_period & 0xFFFFFF;

	return( timebase_freq / chan_period );

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

