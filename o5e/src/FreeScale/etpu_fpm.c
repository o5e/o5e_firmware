/**************************************************************************
 * FILE NAME: $RCSfile: etpu_fpm.c,v $       COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the ETPU Frequency Pulse Measurement (FPM) API      *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_fpm.c,v $
 * Revision 2.0  2005/01/13 16:52:06  r47354
 * Change revision to 2.0
 *
 * Revision 1.4  2005/01/13 15:08:46  r47354
 * Fix CW compiler warnings.
 *
 * Revision 1.3  2005/01/13 15:00:21  r47354
 * Formatting changes.
 *
 * Revision 1.2  2005/01/13 14:32:52  r47354
 * Post review updates.
 *
 * Revision 1.1  2005/01/13 14:02:35  r47354
 * Initial revision.
 *
 *........................................................................*
 *************************************************************************/
#include "etpu_util.h"  /* Utility routines for working with the eTPU */
#include "etpu_fpm.h"   /* eTPU FPM API */
extern uint32_t fs_etpu_data_ram_start;
/********************************************************************************
  FUNCTION  : fs_etpu_fpm_init                                                  
  PURPOSE   : This function detects a periodic signal on an eTPU input pin and  
              returns the pusles in a time window or the frequency.             
                                                                                
  PURPOSE   : To initialize an eTPU channel to detect a periodic input.         
  INPUTS    : This function has 6 parameters:                                   
              channel - channel to run FPM function	                            			
              priority - Scheduler priority: low, middle, or high               
              mode - Single Window or continuous mode                           
              edge - Active Edge to be counted, rising or falling.
              timebase - the timebase/counter register to use - TCR1 or TCR2.          
              window_size - The size of the window in TCR counts                
                                                                                
  RETURNS   : error code                                                        
********************************************************************************/
int16_t fs_etpu_fpm_init (uint8_t channel, uint8_t priority, uint8_t mode,
                  uint8_t edge, uint8_t timebase, uint32_t window_size)
{
    uint32_t *pba;   /*parameter base address for channel */
    uint8_t *pba8;   /*parameter base address for channel -byte indexed */

/* disable channel before changing anything else */
    fs_etpu_disable (channel);

    if (eTPU->CHAN[channel].CR.B.CPBA == 0)
      {
          /* get parameter RAM
             number of parameters passed from eTPU C code */
          pba = fs_etpu_malloc (FS_ETPU_FPM_NUM_PARMS);

          if (pba == 0)
            {
                return (FS_ETPU_ERROR_MALLOC);
            }
      }
    else                        /*set pba to what is in the CR register */
      {
          pba = fs_etpu_data_ram (channel);
      }

    pba8 = (uint8_t *) pba;

    *(pba + ((FS_ETPU_FPM_WINDOW_OFFSET - 1) >> 2)) = (uint32_t) window_size;
    *(pba8 + ((FS_ETPU_FPM_TIMEBASE_OFFSET))) = (uint8_t) timebase;

/* write hsr */
    eTPU->CHAN[channel].HSRR.R = FS_ETPU_FPM_INIT;

/* write FM bits */
    eTPU->CHAN[channel].SCR.R = (uint32_t)(mode + (edge << 1));

/* write CR register */
    eTPU->CHAN[channel].CR.R =
        (priority << 28) + (FS_ETPU_FPM_TABLE_SELECT << 24) +
        (FS_ETPU_FPM_FUNCTION_NUMBER << 16) + (((uint32_t) pba -
        fs_etpu_data_ram_start) >> 3);
    return (0);
}

/*******************************************************************************
  FUNCTION  : fs_etpu_fpm_update_window_size
  PURPOSE   : To update the window parameter for FPM function on a specific
              channel.                                                          
  INPUTS    : This function has 3 parameters:                                   
              channel - channel to run FPM function                             
              window - The period of the FPM in terms of number of TCR1 counts. 
  RETURNS   : error code                                                        
********************************************************************************/
void fs_etpu_fpm_update_window_size (uint8_t channel, uint32_t window)
{
    uint32_t *pba;              /*parameter base address for channel */

    pba = fs_etpu_data_ram (channel);

    *(pba + ((FS_ETPU_FPM_WINDOW_OFFSET - 1) >> 2)) = (uint32_t) window;

    eTPU->CHAN[channel].HSRR.R = FS_ETPU_FPM_UPDATE;

}

/*******************************************************************************
  FUNCTION  : fs_etpu_fpm_get_count                                             
  PURPOSE   : To update the window parameter for FPM function on a specific     
              channel.                                                          
  INPUTS    : This function has 3 parameters:                                   
              channel - channel to run FPM function                             
  RETURNS   : count - the number of edges count by the FPM function             
********************************************************************************/
uint32_t fs_etpu_fpm_get_count (uint8_t channel)
{
    uint32_t *pba_ext;          /*sign extended parameter base address for channel */
    uint32_t count;

    pba_ext = fs_etpu_data_ram (channel) + 0x1000;

    count = *(pba_ext + ((FS_ETPU_FPM_EDGE_COUNT_OFFSET - 1) >> 2));

    return (count);
}

/*******************************************************************************
  FUNCTION  : fs_etpu_fpm_get_freq                                              
  PURPOSE   : To update the window parameter for FPM function on a specific     
              channel.                                                          
  INPUTS    : This function has 3 parameters:                                   
              channel - channel to run FPM function                             
              timebase_freq - the frequency of the selected TCR                 
  RETURNS   : freq - the frequency of the measured pulses                       
********************************************************************************/
uint32_t fs_etpu_fpm_get_freq (uint8_t channel, uint32_t timebase_freq)
{
    uint32_t *pba_ext;          /*sign extended parameter base address for channel */
    unsigned long long count_period;    /* elapsed TCR counts */
    unsigned long long edge_count;      /* number of edges measured */
    uint32_t freq;
    unsigned long long freq0, freq_add;

    /* 
       long long (64 bits variables are used in this API; this is because 
       the result of the calculation :

       edge_count /count_period) * timebase_freq

       may well overflow under a variety of circumstamces.
       Floats are not supported as these may be dealt with differently on 
       different hardware sets.
     */

    pba_ext = fs_etpu_data_ram (channel) + 0x1000;

    /* Get values from Data RAM  */
    edge_count = *(pba_ext + ((FS_ETPU_FPM_EDGE_COUNT_OFFSET - 1) >> 2));
    count_period =
        *(pba_ext + ((FS_ETPU_FPM_ELAPSED_TCR_COUNTS_OFFSET - 1) >> 2));

    /* calculate Frequency */
    freq0 =
        (volatile unsigned long long) ((edge_count / count_period) *
                                       timebase_freq);
    freq_add = (edge_count % count_period) * timebase_freq / count_period;
    /* freq_add catches any remainder from the integer division of 
       edge_count /count_period; So any rounding error is acommodated.
     */

    freq = (uint32_t) (freq0 + freq_add);

    return (freq);
}

/*********************************************************************
 *
 * Copyright:
 *	FREESCALE, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is
 *  retained without alteration in any modified and/or redistributed
 *  versions, and that such modified versions are clearly identified
 *  as such. No licenses are granted by implication, estoppel or
 *  otherwise under any patents or trademarks of Motorola, Inc. This
 *  software is provided on an "AS IS" basis and without warranty.
 *
 *  To the maximum extent permitted by applicable law, FREESCALE
 *  DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING
 *  IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
 *  PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH REGARD TO THE
 *  SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF) AND ANY
 *  ACCOMPANYING WRITTEN MATERIALS.
 *
 *  To the maximum extent permitted by applicable law, IN NO EVENT
 *  SHALL FREESCALE BE LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING
 *  WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS
 *  INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY
 *  LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.
 *
 *  Freescale assumes no responsibility for the maintenance and support
 *  of this software
 ********************************************************************/
