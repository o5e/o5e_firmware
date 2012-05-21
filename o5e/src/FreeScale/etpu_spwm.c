/**************************************************************************
 * FILE NAME: $RCSfile: etpu_spwm.c,v $      COPYRIGHT (c) FREESCALE 2006 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the eTPU Synchronised Pulse Width Modulation (SPWM) *
 * API.                                                                   *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_spwm.c,v $
 * Revision 2.0  2006/01/13 11:21:54  r47354
 * no message
 *
 * Revision 1.6  2006/01/09 15:33:05  r47354
 * Post review changes.
 *
 * Revision 1.5  2005/12/23 11:33:47  r47354
 * Fix issue with reference pointer.
 * Select correct engine's timebase dependant upon channel number.
 *
 * Revision 1.4  2005/12/22 16:57:32  r47354
 * Various fixes relating to references.
 *
 * Revision 1.3  2005/12/22 14:32:20  r47354
 * Changes for GCT compliance.
 * Rename fs_etpu_spwm_config_master to fs_etpu_spwm_init_master.
 * Correct a  couple of typos in the comments.
 * Use long longs in delay_in_ticks calculation.
 *
 * Revision 1.2  2005/12/07 16:00:17  r47354
 * Remove coherent update feature.
 *
 * Revision 1.1  2005/12/07 15:27:17  r47354
 * *** empty log message ***
 *
 **************************************************************************/
#include "etpu_util.h"		    /* Utility routines for working with the eTPU */
#include "etpu_spwm.h"		  	/* eTPU SPWM API defines */
extern uint32_t fs_etpu_data_ram_start;

/******************************************************************************
FUNCTION     : fs_etpu_spwm_init_master
PURPOSE      : To initialize an eTPU channel to generate a master SPWM output.
INPUTS NOTES : This function has the following parameters:
               channel - This is the channel number.
                         0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
                  freq - This is the frequency of the SPWM.  The 
                         range of this parameter is determined by the complete
                         system but normally would be between 1Hz-100kHz.
                  duty - This is the initial duty cycle of the PWM. This is a
                         uint16_t with a range of 0-10000. To represent 0-100%
                         with 0.01% resolution.
              timebase - This is the timer to use as a reference for the SPWM
                         signal. This parameter should be assigned to a value
                         of: FS_ETPU_TCR1 or FS_ETPU_TCR2.
         timebase_freq - This is the frequency of the selected timebase.
                         The range of this is the same as the range of the 
                         timebase frequency on the device. This parameter is a 
                         uint32_t.
        reference_mode - This determines whether the function will run in 
                         immediate mode or relative to a timing reference 
                         stored in PRAM.
                         This parameter should be assigned a value of:
                         FS_ETPU_SPWM_REFERNCE_IN_PRAM,
                         FS_ETPU_SPWM_IMMEDIATE 
        *reference_ptr - This is the address of the reference when 
                         reference_mode is FS_ETPU_SPWM_REFERNCE_IN_PRAM.
INT_DMA_on_falling_edge - Determines if an inteerupt/DMA request is generated 
                         on active edge or not. This parameter should be 
                         assigned a value of:
                         FS_SPWM_NO_ACTIVE_EDGE_INT_DMA,
                         FS_SPWM_ACTIVE_EDGE_INT_DMA
                 link1 - This is a packed 32 bit parameter with 4 8 bit 
                          channel numbers in it.
                 link2 - This is a packed 32 bit parameter with 4 8 bit 
                          channel numbers in it.
RETURNS NOTES: Error code if channel could not be initialized. Error code that
               can be returned are: FS_ETPU_ERROR_MALLOC ,FS_ETPU_ERROR_FREQ.
WARNING      : *This function does not configure the pin only the eTPU. In a
               system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_spwm_init_master ( uint8_t channel,
                                     uint32_t freq,
                                     uint16_t duty, 
                                     uint8_t  timebase,
                                     uint32_t timebase_freq,
                                     uint8_t  reference_mode,
                                     uint32_t *reference_ptr,
                                     uint8_t  INT_DMA_on_falling_edge,
                                     uint32_t link1,
                                     uint32_t link2
																   )
{
    uint32_t *pba;	/* parameter base address for channel */
    uint32_t chan_period;
    uint8_t *pba8;
    uint8_t *reference_ptr8;

	/* Disable channel to assign function safely */
	fs_etpu_disable( channel );

    if (eTPU->CHAN[channel].CR.B.CPBA == 0 )
    {
    	/* get parameter RAM for
    	number of parameters passed from eTPU C code */
    	pba = fs_etpu_malloc(FS_ETPU_SPWM_NUM_PARMS );
			
			if (pba == 0)
    	{
    		return (FS_ETPU_ERROR_MALLOC);
    	}
    }
    else /*set pba to what is in the CR register*/
    {
    	pba=fs_etpu_data_ram(channel);
    }
    pba8 = (uint8_t *) pba;
    
	eTPU->CHAN[channel].CR.B.CPBA = (((uint32_t)pba - fs_etpu_data_ram_start)>>3);


    /* Determine frequency of output waveform */
	chan_period = timebase_freq / freq;

	if ((chan_period == 0) || (chan_period > 0x007FFFFF ))
		return( FS_ETPU_ERROR_FREQ);

	/* write parameters to data memory */
	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_PERIOD_OFFSET, 
														 chan_period  );


	if (duty < 10000)
	{
	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_ACTIVE_OFFSET, 
	                           (chan_period * duty) / 10000 );		
	}
	else /*if duty > 100% make duty 100% */
	{

	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_ACTIVE_OFFSET, 
														 chan_period  );		
	}
	
	*(pba + ((FS_ETPU_SPWM_LINK1_OFFSET)>>2))  = link1;
	*(pba + ((FS_ETPU_SPWM_LINK2_OFFSET)>>2))  = link2;
	
	/* populate the reference */
	/* write REF_MODE to master channel */
	/* this is used by the rum_master API to determine how to calculate the */
	/* first edge time */
	if ( reference_mode == FS_ETPU_SPWM_IMMEDIATE)
	{
	    *(pba8 + FS_ETPU_SPWM_REF_MODE) = 0;
			/* generate pointer to the reference for immediate mode*/
			/* The reference is populated by the fs_etpu_rum_spwm API with the */
			/* then current value of the selected TCR */
			
	    reference_ptr8 = pba8 + FS_ETPU_SPWM_REFERENCE;
	    reference_ptr =  (uint32_t *)reference_ptr8 -
        (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4));;
			
    }
		else /* reference mode; ptr is in PRAM */
		{
    *(pba8 + FS_ETPU_SPWM_REF_MODE) = 1;
    /* reference_ptr has been passed in ; no need to calculate */
  }

      /* populate the pointer on the eTPU */
			*(pba + ((FS_ETPU_SPWM_REF_PTR_OFFSET)>>2)) = 
				(uint24_t) reference_ptr;

			
	/* write FM (function mode) bits */
	eTPU->CHAN[channel].SCR.R = (timebase << 1) + INT_DMA_on_falling_edge;

 /* write channel configuration register */
	eTPU->CHAN[channel].CR.R = 
				(FS_ETPU_SPWM_TABLE_SELECT << 24) +
	      (FS_ETPU_SPWM_FUNCTION_NUMBER << 16) +
	      (((uint32_t)pba - fs_etpu_data_ram_start)>>3);

	return(0);
}
/******************************************************************************
FUNCTION     : fs_etpu_spwm_run_master
PURPOSE      : To set the SPWM master channel running.
INPUTS NOTES : This function has the following parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
              priority - This is the priority to assign to the channel.
                         This parameter should be assigned a value of:
                         FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                         FS_ETPU_PRIORITY_LOW.
RETURNS NOTES: none

******************************************************************************/                       
void fs_etpu_spwm_run_master ( uint8_t channel,
                               uint8_t priority )
{
  uint32_t cpba;	/* CPBA field */
  uint32_t *pba;
  uint32_t *reference_ptr;
  uint8_t *pba8;
  uint32_t *host_ref_ptr;
  uint32_t chan_period;
  uint32_t time_now;

		cpba = eTPU->CHAN[channel].CR.B.CPBA;
	
    pba=fs_etpu_data_ram(channel);
    pba8 = (uint8_t *) pba;
    
	chan_period = *(pba + ((FS_ETPU_SPWM_PERIOD_OFFSET )>>2)) ;

	if ( *(pba8 + FS_ETPU_SPWM_REF_MODE) == 0 ) /* immediate mode */
	{
    host_ref_ptr =  (pba + (FS_ETPU_SPWM_REFERENCE ) / 4) ; 
		if ( eTPU->CHAN[channel].SCR.B.FM1 == FS_ETPU_TCR1)
		{
			if (channel < 32) /*engine A channel */
			{
				time_now = eTPU->TB1R_A.R;
			}
			else /*engine B channel */
			{
				time_now = eTPU->TB1R_B.R;
			}
		
		}
		else /*tcr2 timebase */
		{
			if (channel < 32) /*engine A channel */
			{
				time_now = eTPU->TB2R_A.R;
			}
			else /*engine B channel */
			{
				time_now = eTPU->TB2R_B.R;
			}
		}
	*host_ref_ptr = time_now;
	}
	else /* reference is in PRAM */
	{
		/* do nothing; ref address already populated */
	}	
 
	/* write hsr to start channel running */
	eTPU->CHAN[channel].HSRR.R = FS_ETPU_SPWM_INIT_MASTER;

 /* write channel configuration register ; priority bit */

	eTPU->CHAN[channel].CR.B.CPR = (priority);
}
/******************************************************************************
FUNCTION     : fs_etpu_spwm_init_slave
PURPOSE      : To initialize an eTPU channel to generate a slave SPWM output.
INPUTS NOTES : This function has the following parameters:
                 channel - This is the channel number.
                           0-31 for FS_ETPU_A and 64-95 for FS_ETPU_B.
                priority - This is the priority to assign to the channel.
                           This parameter should be assigned a value of:
                           FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
                           FS_ETPU_PRIORITY_LOW.
                    freq - This is the frequency of the SPWM.  The 
                           range of this parameter is determined by the 
                           complete system but normally would be between 
                           1Hz-100kHz.
                    duty - This is the initial duty cycle of the PWM. This is a
                           uint16_t with a range of 0-10000. To represent 
                           0-100% with 0.01% resolution.                  
                   delay - This is the delay in micro-seconds of the rising 
                           edge of the slave relative to the rising edge of the 
                           master or reference.
                timebase - This is the timer to use as a reference for the SPWM
                           signal. This parameter should be assigned to a value
                           of: FS_ETPU_TCR1 or FS_ETPU_TCR2.
           timebase_freq - This is the frequency of the selected timebase.
                           The range of this is the same as the range of the 
                           timebase frequency on the device. This parameter is
                           a uint32_t.
     *MasterFrameEdgePtr - The address of the variable on the eTPU which stores 
                           the time of the next rising edge.
                           
        NOTES: In this case the freq varialble is used only to gerarate the
               TCR counts pertaining to duty and delay. Freq is not used to 
               generate a chan_period variabke as in the init_master function.
RETURNS NOTES: Error code if channel could not be initialized. Error code that
               can be returned are: FS_ETPU_ERROR_MALLOC ,FS_ETPU_ERROR_FREQ.
WARNING      : This function does not configure the pin only the eTPU. In a
               system a pin may need to be configured to select the eTPU.
******************************************************************************/
int32_t fs_etpu_spwm_init_slave ( uint8_t  channel,
                                  uint8_t  priority,
                                  uint32_t freq,
                                  uint16_t duty,                                  
                                  uint32_t delay,
                                  uint8_t  timebase, 
                                  uint32_t timebase_freq,
                                  uint32_t *MasterFrameEdgePtr
																)
{
    uint32_t *pba;	/* parameter base address for channel */
    uint32_t chan_period;
		uint32_t delay_in_ticks;
		unsigned long long x;

	/* Disable channel to assign function safely */
	fs_etpu_disable( channel );

    if (eTPU->CHAN[channel].CR.B.CPBA == 0 )
    {
        /* get parameter RAM
        number of parameters passed from eTPU C code */
        pba = fs_etpu_malloc(FS_ETPU_SPWM_NUM_PARMS);

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
		
	eTPU->CHAN[channel].CR.B.CPBA = 
		(((uint32_t)pba - fs_etpu_data_ram_start)>>3);

	/* write parameters to data memory */
	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_PERIOD_OFFSET, 
														 chan_period  );

	/* write parameters to data memory */
	if (duty < 10000)
	{
	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_ACTIVE_OFFSET, 
	                           (chan_period * duty) / 10000 );		
	}
	else /*if duty > 100% make duty 100% */
	{

	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_ACTIVE_OFFSET, 
														 chan_period  );		
	}

	x = ( unsigned long long )delay * timebase_freq;
	
	delay_in_ticks = (uint32_t)(x / 1000000);
			/*need to very this can't overflow */
	
	*(pba + ((FS_ETPU_SPWM_DELAY_OFFSET )>>2))  = 
			delay_in_ticks  ;

  *(pba + ((FS_ETPU_SPWM_MASTERRISINGEDGEPTR )>>2)) =
  (uint32_t )MasterFrameEdgePtr;	
  
	/* write hsr to start channel running */
	eTPU->CHAN[channel].HSRR.R = FS_ETPU_SPWM_INIT_SLAVE;
 
			
	/* write FM (function mode) bits */
	eTPU->CHAN[channel].SCR.R = (timebase << 1) + 0;
	/* slave cannot generate interrupts */
	
	 /* write channel configuration register */
	eTPU->CHAN[channel].CR.R = (priority << 28) + 
				(FS_ETPU_SPWM_TABLE_SELECT << 24) +
	      (FS_ETPU_SPWM_FUNCTION_NUMBER << 16) +
	      (((uint32_t)pba - fs_etpu_data_ram_start)>>3);

	return(0);
}

/******************************************************************************
FUNCTION     : fs_etpu_spwm_duty
PURPOSE      : To update a channel duty cycle using a 16 bit integer value. The
               integer value is the percentage *100, so 20% would be 2000.
               Supports both master and slave channels.
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
                  duty - This is the duty cycle of the PWM. This is a
                         uint16 with a range of 0-10000. To represent 0-100%
                         with 0.01% resolution.
RETURNS NOTES: none
WARNING      :
******************************************************************************/
void fs_etpu_spwm_duty( uint8_t channel, 
                        uint16_t duty)
{
	uint32_t *pba;
	uint32_t chan_period;

	pba = fs_etpu_data_ram (channel);

	chan_period = *(pba + FS_ETPU_SPWM_PERIOD_OFFSET-1);
	chan_period = chan_period & 0xFFFFFF;

	if (duty < 10000)
	{
	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_ACTIVE_OFFSET, 
	                           (chan_period* duty) / 10000 );		
	}
	else /*if duty > 100% make duty 100% */
	{

	fs_etpu_set_chan_local_24( channel,  FS_ETPU_SPWM_ACTIVE_OFFSET, chan_period  );		
	}

}
/******************************************************************************
FUNCTION     : fs_etpu_spwm_update_master
PURPOSE      : To update a PWM output's frequency and dutycycle
INPUTS NOTES : This function has the following parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
                  freq - This is the frequency of the SPWM. The range of
                         this parameter is determine by the complete system but
                         normally would be between 1Hz-100kHz.
                  duty - This is the initial duty cycle of the PWM. This is a
                         uint16_t with a range of 0-10000. To represent 0-100%
                         with 0.01% resolution.
         timebase_freq - This is the frequency of the selected timebase.
                         The range of this is the same as the range of the 
                         timebase frequency on the device. This parameter is a 
                         uint32_t.
RETURNS NOTES: Error code if frequency is out of range: FS_ETPU_ERROR_FREQ
******************************************************************************/
int32_t fs_etpu_spwm_update_master( uint8_t channel,
                                    uint32_t freq,
                                    uint16_t duty,
                                    uint32_t timebase_freq
                                    )
{
	uint32_t *pba;
	uint32_t chan_period;

	pba = fs_etpu_data_ram (channel);

    /* Determine frequency of output waveform */
	chan_period = timebase_freq / freq;

	if ((chan_period == 0) || (chan_period > 0x007FFFFF ))
		return( FS_ETPU_ERROR_FREQ);

	/* write parameters to data memory */
	*(pba + ((FS_ETPU_SPWM_PERIOD_OFFSET )>>2)) = chan_period;
	*(pba + ((FS_ETPU_SPWM_ACTIVE_OFFSET )>>2))  = (chan_period * duty) / 10000;


}

/******************************************************************************
FUNCTION     : fs_etpu_spwm_get_freq
PURPOSE      : To determine the actual frequency by the SPWM channel.
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                         0-31 for ETPU_A and 64-95 for ETPU_B.
       timebase_freq -   This is the frequency of the selected timebase.
                         The range of this is the same as the range of the 
                         timebase frequency on the device. This parameter 
                         is a uint32_t.
RETURNS NOTES: The actual frequency as an integer.
WARNING      :
******************************************************************************/
uint32_t fs_etpu_spwm_get_freq_master( uint8_t channel, 
                                       uint32_t timebase_freq
                                     )
{
	uint32_t chan_period;
	uint32_t *pba;

	pba = fs_etpu_data_ram (channel);

	chan_period = *(pba + ((FS_ETPU_SPWM_PERIOD_OFFSET )>>2));
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

