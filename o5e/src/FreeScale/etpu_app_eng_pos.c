/**************************************************************************
 * FILE NAME: $RCSfile: etpu_app_eng_pos.c,v $                            *
 *                                           COPYRIGHT (c) FREESCALE 2006 *
 *                                                  All Rights Reserved   *
 * DESCRIPTION:                                                           *
 * This file contains the eTPU Engine Position (eng_pos) API.             *
 *========================================================================*
 * ORIGINAL AUTHOR: Geoff Emerson (r47354)                                *
 * $Log: etpu_app_eng_pos.c,v $
 * Revision 1.6  2008/11/26 15:29:36  r47354
 * Add type for parameter in speed API.
 *
 * Revision 1.5  2008/11/13 10:49:10  r47354
 * Added fs_etpu_eng_pos_get_engine_speed function.
 *
 * Revision 1.4  2008/08/28 13:22:19  r47354
 * Fix issues with crank_blank_time_ms and crank_first_tooth_timeout_us calculations and limiting.
 * Make fs_etpu_eng_pos_get_engine_cycle_0_count fs_etpu_eng_pos_get_engine_cycle_0_tvr2_count
 * fs_etpu_eng_pos_insert_tooth now has host_asserted_tooth_count parameter.
 *
 * Revision 1.3  2008/06/23 14:22:55  r47354
 * Added range checking.
 * Added code for fs_etpu_eng_pos_set_wr_after_gap, fs_etpu_eng_pos_set_wr_across_gap, fs_etpu_eng_pos_set_wr_timeout
 *
 * Revision 1.2  2008/06/06 09:53:02  r47354
 * Major rework based on etpuc code changes and review.
 *
 * Revision 1.1  2008/01/21 13:49:05  r47354
 * Initial revision
 *
 *                                                                        *
 **************************************************************************/
#include "etpu_util.h"		    /* Utility routines for working with the eTPU */
#include "etpu_app_eng_pos.h"		  	  /* eTPU Engine Position API defines */
extern uint32_t fs_etpu_data_ram_start;

/**************************************************************************
 * 
 * Global Variables
 *
 **************************************************************************/

uint8_t g_crank_channel, g_cam_channel;

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif

/******************************************************************************
FUNCTION     : fs_etpu_app_eng_pos_init
PURPOSE      : To initialize 2 eTPU channels to for the synchronisation 
               functions.
INPUTS NOTES : This function has the following parameters:
           cam_channel - This is the CAM channel number.
                         1-31 for FS_ETPU_A and 65-95 for FS_ETPU_B.               
          cam_priority - This is the priority to assign to the cam channel.
                         This parameter should be assigned a value of:
                         FS_ETPU_CAM_PRIORITY_HIGH, FS_ETPU_PRIORITY_CAM_MIDDLE 
                         or FS_ETPU_CAM_PRIORITY_LOW.
     cam_edge_polarity - The polarity if the CAM edge. Can be 
                         FS_ETPU_CAM_FM0_RISING_EDGE or 
                         FS_ETPU_CAM_FM0_FALLING_EDGE           
cam_angle_window_start - The starting postion of the CAM window.
                         Range 0 to 71999. 71999 represents 719.99 degress
cam_angle_window_width - The width of the CAM window.
                         Range 0 to 71999. 71999 represents 719.99 degress
         crank_channel - This is the CRANK channel number.
                         0 for ETPU_A or 64 for ETPU_B.
        crank_priority - This is the priority to assign to the crank channel.
                         This parameter should be assigned a value of:
                         FS_ETPU_CRANK_PRIORITY_HIGH, 
                         FS_ETPU_CRANK_PRIORITY_MIDDLE
                         or FS_ETPU_CRANK_PRIORITY_LOW.      
   crank_edge_polarity - The polarity if the CRANK edge. Can be 
                         FS_ETPU_CRANK_FM0_RISING_EDGE or
                         FS_ETPU_CRANK_FM0_FALLING_EDGE.
crank_number_of_physical_teeth - number of physical teeth on the crank wheel. 
                         Can be 1 to 255.
crank_number_of_missing_teeth - number of missing teeth on the crank wheel.
                         Can be 1,2 or 3.
crank_blank_tooth_count - Number of teeth to be ignored after first active edge.
                         Can be 0 to 255.
crank_tcr2_ticks_per_tooth - Number of TCR2 counts per tooth. Can be 1 to 1024.   
crank_windowing_ratio_normal - A fractional number. Used to window out noise. 
                         Can be between 0x0 and 0xFFFFFF. Applied to 'noraml 
                         teeth' - ratios below are used for other teeth.                         
crank_windowing_ratio_after_gap - A fractional number. Used to window out noise. 
                         Can be between 0x0 and 0xFFFFFF. Applied to the 
                         second tooth after the gap.                         
crank_windowing_ratio_across_gap - A fractional number. Used to window out  
                         noise. Can be between 0x0 and 0xFFFFFF. Applied to the 
                         the first tooth after the gap.                         
crank_windowing_ratio_timeout - A fractional number. Used to window out noise. 
                         Can be between 0x0 and 0xFFFFFF. Applied to the tooth 
                         immediately following a timeout.
       crank_gap_ratio - A fractional number. Used to identify the gap. Can be  
                         between 0x0 and 0xFFFFFF.
   crank_blank_time_ms - The amount of time that teeth are ignored for at start 
                         up in milli seconds. crank_blank_time_ms has a 
                         maximum value given by:
                         ( 0xFFFFFF/tcr1_timebase_freq ) * 1000.         
crank_first_tooth_timeout_us - The amount of time in micro seconds after which  
                         the first tooth will be deemed to have have timed out.
                         Maximum value given by:
                         ( 0xFFFFFF/tcr1_timebase_freq ) * 1000000
          crank_link_1 - This is a packed 32 bit parameter with 4 8 bit 
                         channel numbers in it. These channels are signalled 
                         when CRANK has had to re-sync (e.g. STALL etc) 
          crank_link_2 - This is a packed 32 bit parameter with 4 8 bit 
                         channel numbers in it. These channels are signalled 
                         when CRANK has had to re-sync (e.g. STALL etc)                               
          crank_link_3 - This is a packed 32 bit parameter with 4 8 bit 
                         channel numbers in it. These channels are signalled 
                         when CRANK has had to re-sync (e.g. STALL etc)                               
          crank_link_4 - This is a packed 32 bit parameter with 4 8 bit 
                         channel numbers in it. These channels are signalled 
                         when CRANK has had to re-sync (e.g. STALL etc)                               
    tcr1_timebase_freq - The frequency of the TCR1 timebase in Hz. 
RETURNS NOTES: Error code if channel could not be initialized. Error code that
               can be returned are: FS_ETPU_ERROR_MALLOC , FS_ETPU_ERROR_VALUE.
WARNING      : *This function does not configure the pin only the eTPU. In a
               system a pin may need to be configured to select the eTPU.
******************************************************************************/
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
                                  uint32_t tcr1_timebase_freq                                  
                            )
{
    uint32_t *cam_pba;	 /* parameter base address for CAM channel */
    uint32_t *crank_pba;	 /* parameter base address for CRANK channel */
    uint32_t cam_angle_window_start_tcr2_count;
    uint32_t cam_angle_window_width_tcr2_count;
     uint8_t *crank_pba8; /* pointer for 8 bit crank variables */
     uint8_t *cam_pba8;  /* pointer for 8 bit crank variables */
    uint32_t *crank_state_ptr;
    uint32_t *crank_cam_state_ptr; /* pointer so CAM can see Crank_State */
    uint32_t tcr2_counts_per_engine_cycle; /* number of tcr2 counts 
                                                  in an engine cycle */
    uint32_t crank_blank_time;
    uint32_t crank_first_tooth_timeout; 
    
  int32_t error_code = 0; /* returned value from etpu API functions */

  g_crank_channel = crank_channel;
  g_cam_channel = cam_channel;

	/* Disable CAM channel to assign function safely */
	fs_etpu_disable( cam_channel );
	/* Disable CRANK channel to assign function safely */
	fs_etpu_disable( crank_channel );

    if (eTPU->CHAN[cam_channel].CR.B.CPBA == 0 )
    {
    	/* get parameter RAM for
    	number of parameters passed from eTPU C code */
    	cam_pba = fs_etpu_malloc(FS_ETPU_CAM_NUM_PARMS );
			
			if (cam_pba == 0)
    	{
    		return (FS_ETPU_ERROR_MALLOC);
    	}
    }
    else /*set pba to what is in the CR register*/
    {
    	cam_pba=fs_etpu_data_ram(cam_channel);
    }
    cam_pba8 = (uint8_t *) cam_pba;


    if (eTPU->CHAN[crank_channel].CR.B.CPBA == 0 )
    {
    	/* get parameter RAM for
    	number of parameters passed from eTPU C code */
    	crank_pba = fs_etpu_malloc(FS_ETPU_CRANK_NUM_PARMS );
			
			if (crank_pba == 0)
    	{
    		return (FS_ETPU_ERROR_MALLOC);
    	}
    }
    else /*set pba to what is in the CR register*/
    {
    	crank_pba=fs_etpu_data_ram(crank_channel);
    }
    crank_pba8 = (uint8_t *) crank_pba;
        
	eTPU->CHAN[cam_channel].CR.B.CPBA = (((uint32_t)cam_pba -  fs_etpu_data_ram_start)>>3);
	eTPU->CHAN[crank_channel].CR.B.CPBA = (((uint32_t)crank_pba -  fs_etpu_data_ram_start)>>3);
	
	/* Parameter range chacking */
	
  if (cam_angle_window_start > 72000)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_angle_window_start = 72000;
  }
  
  if (cam_angle_window_width > 72000)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_angle_window_width = 72000;
  }
  
  if (crank_tcr2_ticks_per_tooth > 1024)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_tcr2_ticks_per_tooth = 1024;
  }

  if (crank_windowing_ratio_normal > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_windowing_ratio_normal = 0xFFFFFF;
  }
  
  if (crank_windowing_ratio_after_gap > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_windowing_ratio_after_gap = 0xFFFFFF;
  }
  
  if (crank_windowing_ratio_across_gap > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_windowing_ratio_across_gap = 0xFFFFFF;
  }
  
  if (crank_windowing_ratio_timeout > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_windowing_ratio_timeout = 0xFFFFFF;
  }  
  
  if (crank_gap_ratio > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_gap_ratio = 0xFFFFFF;
  }
  
  if (crank_blank_time_ms >  ((long long)0xFFFFFF * 1000 )/(long long)tcr1_timebase_freq)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_blank_time_ms =  ((long long)0xFFFFFF * 1000 )/(long long)tcr1_timebase_freq;
  }

  if (crank_first_tooth_timeout_us >  ((long long)0xFFFFFF * 1000000)/(long long)tcr1_timebase_freq)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_first_tooth_timeout_us =  ((long long)0xFFFFFF * 1000000)/(long long)tcr1_timebase_freq;
  }
                 
 
  /* calculate eTPU function parameters based on API input parameters */
  tcr2_counts_per_engine_cycle = crank_tcr2_ticks_per_tooth * ((crank_number_of_physical_teeth + crank_number_of_missing_teeth) *2);
  cam_angle_window_start_tcr2_count = ((long long)cam_angle_window_start * tcr2_counts_per_engine_cycle)/(long long)72000;
  cam_angle_window_width_tcr2_count = ((long long)cam_angle_window_width * tcr2_counts_per_engine_cycle)/(long long)72000; 

    crank_cam_state_ptr = cam_pba 
                    -  (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4)) 
                   + (FS_ETPU_CAM_STATE_OFFSET >>2); 

	/* write parameters to data memory for CAM */

   *(cam_pba + ((FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = tcr2_counts_per_engine_cycle;
   *(cam_pba + ((FS_ETPU_CAM_ANGLE_WINDOW_START_NEW_OFFSET - 1)>>2)) = cam_angle_window_start_tcr2_count;
   *(cam_pba + ((FS_ETPU_CAM_ANGLE_WINDOW_WIDTH_NEW_OFFSET - 1)>>2)) = cam_angle_window_width_tcr2_count;
   *(cam_pba + ((FS_ETPU_CAM_ANGLE_START_COUNT_OFFSET - 1)>>2)) = tcr2_counts_per_engine_cycle;  	

   *((uint8_t *)cam_pba + FS_ETPU_CAM_STATE_OFFSET) = (uint8_t)FS_ETPU_CAM_SYNC_SEEK_OR_START_ANGLE;
   *((uint8_t *)cam_pba + FS_ETPU_CAM_ERROR_STATUS_OFFSET) = (uint8_t)FS_ETPU_CAM_NO_ERROR;


	/* write parameters to data memory for CRANK */
  if (crank_number_of_missing_teeth<=3 && crank_number_of_missing_teeth >=1)
  {
  	/* create a mask so the eTPUC can quickly write the  TPR.MISSCNT field */
  	   *(crank_pba + ((FS_ETPU_CRANK_MISSING_TOOTH_COUNT_MASK_OFFSET - 1)>>2)) = crank_number_of_missing_teeth<<13;
  }
  else
  {
    return (FS_ETPU_ERROR_VALUE);
  }
  *(crank_pba + ((FS_ETPU_CRANK_TICKS_PER_TOOTH_OFFSET - 1)>>2)) = crank_tcr2_ticks_per_tooth;  	
  *(crank_pba + ((FS_ETPU_CRANK_WINDOWING_RATIO_NORMAL_OFFSET - 1)>>2)) = crank_windowing_ratio_normal;  	
  *(crank_pba + ((FS_ETPU_CRANK_WINDOWING_RATIO_AFTER_GAP_OFFSET - 1)>>2)) = crank_windowing_ratio_after_gap;  	
  *(crank_pba + ((FS_ETPU_CRANK_WINDOWING_RATIO_ACROSS_GAP_OFFSET - 1)>>2)) = crank_windowing_ratio_across_gap;  	
  *(crank_pba + ((FS_ETPU_CRANK_WINDOWING_RATIO_TIMEOUT_OFFSET - 1)>>2)) = crank_windowing_ratio_timeout;  	
  *(crank_pba + ((FS_ETPU_CRANK_GAP_RATIO_OFFSET - 1)>>2)) = crank_gap_ratio;  	
  *(crank_pba + ((FS_ETPU_CRANK_TICKS_PER_TOOTH_OFFSET - 1)>>2)) = crank_tcr2_ticks_per_tooth;  	
  *(crank_pba + ((FS_ETPU_CRANK_CAM_STATE_OFFSET - 1)>>2)) = (uint24_t)crank_cam_state_ptr;
  *(crank_pba + (FS_ETPU_CRANK_LINK_1_OFFSET >>2)) = crank_link_1;
  *(crank_pba + (FS_ETPU_CRANK_LINK_2_OFFSET >>2)) = crank_link_2;
  *(crank_pba + (FS_ETPU_CRANK_LINK_3_OFFSET >>2)) = crank_link_3;
  *(crank_pba + (FS_ETPU_CRANK_LINK_4_OFFSET >>2)) = crank_link_4;

												
  crank_blank_time = ((long long) crank_blank_time_ms *  tcr1_timebase_freq)/(long long)1000 ;
  /* crank_blank_time_ms has a maximum value given by:
   ( 0xFFFFFF * 1000) / tcr1_timebase_freq
  */
  
  *(crank_pba + ((FS_ETPU_CRANK_BLANK_TIME_OFFSET - 1)>>2)) = crank_blank_time;  	

  crank_first_tooth_timeout = ((long long) crank_first_tooth_timeout_us  * tcr1_timebase_freq) / (long long)1000000;
  /* crank_first_tooth_timeout_us has a maximum value given by:
   ( 0xFFFFFF/tcr1_timebase_freq ) * 1000000
  */
  
  *(crank_pba + ((FS_ETPU_CRANK_FIRST_TOOTH_TIMEOUT_OFFSET - 1)>>2)) = crank_first_tooth_timeout;  	
  *(crank_pba + ((FS_ETPU_CRANK_ANGLE_START_COUNT_OFFSET - 1)>>2)) = tcr2_counts_per_engine_cycle;  	
														
  *((uint8_t *)crank_pba + FS_ETPU_CRANK_PHYSICAL_CRANK_TEETH_OFFSET) = (uint8_t)crank_number_of_physical_teeth;
  *((uint8_t *)crank_pba + FS_ETPU_CRANK_MISSING_TOOTH_COUNT_OFFSET) = (uint8_t)crank_number_of_missing_teeth;
  *((uint8_t *)crank_pba + FS_ETPU_CRANK_BLANK_TOOTH_COUNT_OFFSET) = (uint8_t)crank_blank_tooth_count;
  *((uint8_t *)crank_pba + FS_ETPU_CRANK_ERROR_STATUS_OFFSET) = (uint8_t)FS_ETPU_CRANK_NO_ERROR;
  *((uint8_t *)crank_pba + FS_ETPU_CRANK_STATE_OFFSET) = (uint8_t)FS_ETPU_CRANK_BLANK_TIME;
  *((uint8_t *)crank_pba + FS_ETPU_CRANK_BLANK_TIME_EXPIRED_FLAG_OFFSET) = (uint8_t)0;
													      

	/* Write to data memory - global variable*/
  fs_etpu_set_global_24(FS_ETPU_ENG_POS_SYNC_STATUS_GLOBAL_OFFSET, FS_ETPU_ENG_POS_SEEK);

	/* write FM (function mode) bits */
	eTPU->CHAN[cam_channel].SCR.R = cam_edge_polarity;
	eTPU->CHAN[crank_channel].SCR.R = crank_edge_polarity;
	
	/* update the TBCR TCR2CTL field according to edge polarity */
	if (crank_edge_polarity == FS_ETPU_CRANK_FM0_RISING_EDGE)
	{
		if (crank_channel <=63) /* engine A */
		{
			eTPU->TBCR_A.B.TCR2CTL = FS_ETPU_TCR2CTL_RISE>>29;
		}
		else /* engine B */
		{			
			eTPU->TBCR_B.B.TCR2CTL = FS_ETPU_TCR2CTL_RISE>>29;
		}
	}
	else /* Falling edge */
	{
		if (crank_channel <=63) /* engine A */
		{
			eTPU->TBCR_A.B.TCR2CTL = FS_ETPU_TCR2CTL_FALL>>29;
		}
		else /* engine B */
		{			
			eTPU->TBCR_B.B.TCR2CTL = FS_ETPU_TCR2CTL_FALL>>29;
		}
	}
	
    /*write hsr*/
    eTPU->CHAN[crank_channel].HSRR.R = FS_ETPU_CRANK_INIT;
    eTPU->CHAN[cam_channel].HSRR.R = FS_ETPU_CAM_INIT;
			
 /* write channel configuration registers */
	eTPU->CHAN[cam_channel].CR.R =
	      (cam_priority << 28) + 
				(FS_ETPU_CAM_TABLE_SELECT << 24) +
	      (FS_ETPU_CAM_FUNCTION_NUMBER << 16) +
	      (((uint32_t)cam_pba -  fs_etpu_data_ram_start)>>3);

	eTPU->CHAN[crank_channel].CR.R = 
	      (crank_priority << 28) + 
				(FS_ETPU_CRANK_TABLE_SELECT << 24) +
	      (FS_ETPU_CRANK_FUNCTION_NUMBER << 16) +
	      (((uint32_t)crank_pba -  fs_etpu_data_ram_start)>>3);
	
	return(error_code);
}
int32_t fs_etpu_eng_update_cam_window(uint32_t cam_angle_window_start, 
                                      uint32_t cam_angle_window_width
                                     )
                           
{
  uint32_t cam_angle_window_start_tcr2_count;
  uint32_t cam_angle_window_width_tcr2_count;	
  uint8_t crank_number_of_physical_teeth;
  uint8_t crank_number_of_missing_teeth;
  uint32_t crank_tcr2_ticks_per_tooth;
  uint32_t tcr2_counts_per_engine_cycle;
  int32_t error_code = 0; /* returned value from etpu API functions */

	/* Parameter range chacking */
	
  if (cam_angle_window_start > 72000)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_angle_window_start = 72000;
  }
  
  if (cam_angle_window_width > 72000)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_angle_window_width = 72000;
  }

  crank_number_of_physical_teeth = fs_etpu_get_chan_local_8(g_crank_channel,  FS_ETPU_CRANK_PHYSICAL_CRANK_TEETH_OFFSET);
  
  crank_number_of_missing_teeth = fs_etpu_get_chan_local_8(g_crank_channel,  FS_ETPU_CRANK_MISSING_TOOTH_COUNT_OFFSET);
  
  crank_tcr2_ticks_per_tooth = fs_etpu_get_chan_local_24(g_crank_channel,  FS_ETPU_CRANK_TICKS_PER_TOOTH_OFFSET);

  tcr2_counts_per_engine_cycle = crank_tcr2_ticks_per_tooth * ((crank_number_of_physical_teeth + crank_number_of_missing_teeth) *2);

  cam_angle_window_start_tcr2_count = ((long long)cam_angle_window_start * tcr2_counts_per_engine_cycle)/(long long)72000;
  cam_angle_window_width_tcr2_count = ((long long)cam_angle_window_width * tcr2_counts_per_engine_cycle)/(long long)72000; 

	fs_etpu_set_chan_local_24( g_cam_channel,  
	                           FS_ETPU_CAM_ANGLE_WINDOW_WIDTH_HOST_OFFSET,  
													 	 cam_angle_window_width_tcr2_count);

	fs_etpu_set_chan_local_24( g_cam_channel,  
	                           FS_ETPU_CAM_ANGLE_WINDOW_START_HOST_OFFSET,  
													 	 cam_angle_window_start_tcr2_count );

   eTPU->CHAN[g_cam_channel].HSRR.R = FS_ETPU_CAM_UPDATE_WINDOW;
													 	 
	return(0);
}

/* error handling */
int8_t fs_etpu_eng_pos_get_cam_error_status()
{
	return(fs_etpu_get_chan_local_8( g_cam_channel, 
	                                 FS_ETPU_CAM_ERROR_STATUS_OFFSET));
}

uint32_t fs_etpu_eng_pos_get_cam_edge_angle()
{
  uint8_t crank_number_of_physical_teeth;
  uint8_t crank_number_of_missing_teeth;
  uint32_t crank_tcr2_ticks_per_tooth;
  uint32_t tcr2_counts_per_engine_cycle;
  uint32_t cam_edge_angle;

  crank_number_of_physical_teeth = fs_etpu_get_chan_local_8(g_crank_channel,  FS_ETPU_CRANK_PHYSICAL_CRANK_TEETH_OFFSET);
  
  crank_number_of_missing_teeth = fs_etpu_get_chan_local_8(g_crank_channel,  FS_ETPU_CRANK_MISSING_TOOTH_COUNT_OFFSET);
  
  crank_tcr2_ticks_per_tooth = fs_etpu_get_chan_local_24(g_crank_channel,  FS_ETPU_CRANK_TICKS_PER_TOOTH_OFFSET);

  tcr2_counts_per_engine_cycle = crank_tcr2_ticks_per_tooth * ((crank_number_of_physical_teeth + crank_number_of_missing_teeth) *2);

  cam_edge_angle = ((long long)fs_etpu_get_chan_local_24( g_cam_channel, 
	                                 FS_ETPU_CAM_EDGE_ANGLE_OFFSET) * 72000)/(long long)tcr2_counts_per_engine_cycle;


	return(cam_edge_angle);	
}

int8_t fs_etpu_eng_pos_get_crank_error_status()
{
	return(fs_etpu_get_chan_local_8( g_crank_channel, 
	                                 FS_ETPU_CRANK_ERROR_STATUS_OFFSET));
}

int32_t fs_etpu_eng_pos_clear_cam_error_status()
{
	  fs_etpu_set_chan_local_8( g_cam_channel,  
	                            FS_ETPU_CAM_ERROR_STATUS_OFFSET,  
														  FS_ETPU_CAM_NO_ERROR );
    return(0);
}

int32_t fs_etpu_eng_pos_get_engine_cycle_0_tcr2_count()
{
	  return(fs_etpu_get_chan_local_24( g_cam_channel,  
	                            FS_ETPU_CAM_ENGINE_CYCLE_0_COUNT_OFFSET));
}

int32_t fs_etpu_eng_pos_clear_crank_error_status()
{
  fs_etpu_set_chan_local_8( g_crank_channel,  
	                          FS_ETPU_CRANK_ERROR_STATUS_OFFSET,  
														FS_ETPU_CRANK_NO_ERROR );
    return(0);
}

int8_t fs_etpu_eng_pos_get_cam_status()
{
		return(fs_etpu_get_chan_local_8( g_cam_channel, 
		                                 FS_ETPU_CAM_STATE_OFFSET)); 
}

int8_t fs_etpu_eng_pos_get_crank_status()
{
	return(fs_etpu_get_chan_local_8( g_crank_channel, 
	                                 FS_ETPU_CRANK_STATE_OFFSET)); 
}

int8_t fs_etpu_eng_pos_get_engine_position_status()
{
		return(fs_etpu_get_global_24(FS_ETPU_ENG_POS_SYNC_STATUS_GLOBAL_OFFSET)); 
}

int8_t fs_etpu_eng_pos_get_tooth_number()
{
	return(fs_etpu_get_chan_local_8( g_crank_channel, 
	                                 FS_ETPU_CRANK_TOOTH_COUNT_OFFSET)); 
}

int32_t fs_etpu_eng_pos_set_tooth_number (uint8_t tooth_number )
{
  fs_etpu_set_chan_local_8( g_crank_channel,  
	                          FS_ETPU_CRANK_TOOTH_COUNT_OFFSET,  
														tooth_number );
  return(0);
}

int32_t fs_etpu_eng_pos_get_tooth_time()
{
	return(fs_etpu_get_chan_local_24( g_crank_channel, 
	                                  FS_ETPU_CRANK_TOOTH_TIME_OFFSET));
}
int32_t fs_etpu_eng_pos_get_engine_speed(uint32_t tcr1_timebase_freq)
{
  vuint8_t crank_number_of_physical_teeth;
  vuint8_t crank_number_of_missing_teeth;
  vuint32_t crank_tooth_period;
  vuint32_t speed_rpm;

  crank_number_of_physical_teeth = fs_etpu_get_chan_local_8(g_crank_channel,  FS_ETPU_CRANK_PHYSICAL_CRANK_TEETH_OFFSET);
  crank_number_of_missing_teeth = fs_etpu_get_chan_local_8(g_crank_channel,  FS_ETPU_CRANK_MISSING_TOOTH_COUNT_OFFSET);
  crank_tooth_period = fs_etpu_get_chan_local_24(g_crank_channel,  FS_ETPU_CRANK_TOOTH_PERIOD_A_OFFSET);

  speed_rpm = (tcr1_timebase_freq * 60) /((crank_number_of_physical_teeth+crank_number_of_missing_teeth)*crank_tooth_period) ;
  
  return(speed_rpm);    
}


int32_t fs_etpu_eng_pos_set_wr_normal (ufract24_t ratio )
{
  int32_t error_code = 0; /* returned value from etpu API functions */

  if (ratio > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    ratio = 0xFFFFFF;
  }
  fs_etpu_set_chan_local_24( g_crank_channel,  
	                           FS_ETPU_CRANK_WINDOWING_RATIO_NORMAL_OFFSET,  
														 ratio );
  return(error_code);
}

int32_t fs_etpu_eng_pos_set_wr_after_gap (ufract24_t ratio )
{
  int32_t error_code = 0; /* returned value from etpu API functions */

  if (ratio > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    ratio = 0xFFFFFF;
  }
  fs_etpu_set_chan_local_24( g_crank_channel,  
	                           FS_ETPU_CRANK_WINDOWING_RATIO_AFTER_GAP_OFFSET,  
														 ratio );
  return(error_code);
}

int32_t fs_etpu_eng_pos_set_wr_across_gap (ufract24_t ratio )
{
  int32_t error_code = 0; /* returned value from etpu API functions */

  if (ratio > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    ratio = 0xFFFFFF;
  }
  fs_etpu_set_chan_local_24( g_crank_channel,  
	                           FS_ETPU_CRANK_WINDOWING_RATIO_ACROSS_GAP_OFFSET,  
														 ratio );
  return(error_code);
}

int32_t fs_etpu_eng_pos_set_wr_timeout (ufract24_t ratio )
{
  int32_t error_code = 0; /* returned value from etpu API functions */

  if (ratio > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    ratio = 0xFFFFFF;
  }
  fs_etpu_set_chan_local_24( g_crank_channel,  
	                           FS_ETPU_CRANK_WINDOWING_RATIO_TIMEOUT_OFFSET,  
														 ratio );
  return(error_code);
}

int32_t fs_etpu_eng_pos_insert_tooth(uint8_t host_asserted_tooth_count)
{
	fs_etpu_set_chan_local_8( g_crank_channel, 
	                          FS_ETPU_CRANK_HOST_ASSERTED_TOOTH_COUNT_OFFSET,
	                          host_asserted_tooth_count );

  fs_etpu_set_hsr( g_crank_channel,  FS_ETPU_CRANK_IPH);
  return(0);	
}

int32_t fs_etpu_eng_pos_adjust_angle (int24_t angle_adjust )
{
  fs_etpu_set_chan_local_24( g_crank_channel,  
	                           FS_ETPU_CRANK_SIGNED_ANGLE_ADJUST_OFFSET,  
												     angle_adjust );
  fs_etpu_set_hsr( g_crank_channel,  FS_ETPU_CRANK_ANGLE_ADJUST);
  return(0);	
}

#if __CWCC__
#pragma pop
#endif