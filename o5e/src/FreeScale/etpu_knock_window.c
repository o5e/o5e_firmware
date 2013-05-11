/**************************************************************************
 * FILE NAME: $RCSfile: etpu_knock_window.c,v $                           *
 *                                           COPYRIGHT (c) FREESCALE 2008 *
 *                                                  All Rights Reserved   *
 * DESCRIPTION:                                                           *
 * This file contains the eTPU knock window API.                          *
 *========================================================================*
 * ORIGINAL AUTHOR: David Paterson (r43562)                               *
 * $Log: etpu_knock_window.c,v $
 * Revision 1.2  2008/10/29 16:21:49  r43562
 * Updated number_of_windows to be uint8 (previously uint24) and updated INPUT NOTES in comments above functions.
 *
 * Revision 1.1  2008/06/25 08:00:18  r43562
 * Initial version.
 *                                          *
 *                                                                        *
 **************************************************************************/
#include "etpu_util.h"		  /* Utility routines for working with the eTPU */
#include "etpu_knock_window.h"	 	   	   /* eTPU Knock Window API defines */
extern uint32_t fs_etpu_data_ram_start;

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif

/****************************************************************************
FUNCTION     : fs_etpu_knock_window_init
PURPOSE      : To initialize an eTPU channel to generate a knock window 
INPUTS NOTES : This function has the following parameters:
               channel - The knock window channel number. For products with 
               				a single eTPU, this parameter should be assigned a 
               				value of 0-31. For devices with two eTPUs, this 
               				parameter should be assigned a value of 0-31 for eTPU_A 
               				and 64-95 for eTPU_B.
               priority - The priority to assign to the eTPU IC channel. 
               						The following eTPU priority definitions are found in 
               						utilities file etpu_utils.h.:
													FS_ETPU_PRIORITY_HIGH
													FS_ETPU_PRIORITY_MIDDLE
													FS_ETPU_PRIORITY_LOW
													FS_ETPU_PRIORITY_DISABLED
               number_of_windows - This is the total number of knock 
               										 windows. Range is 1 to 8.
               edge_polarity - This is the polarity of the knock signal.
               							   This parameter should be assigned a value of:
               							   FS_ETPU_KNOCK_FM0_RISING_EDGE or
               							   FS_ETPU_KNOCK_FM0_FALLING_EDGE
               edge_interrupt - This is to generate an interrupt on open or
               									close of knock window. 
               									This parameter should be assigned a value of:
               							    FS_ETPU_KNOCK_FM1_INT_OPEN or
               							    FS_ETPU_KNOCK_FM1_INT_CLOSE
               cam_channel - CAM channel number used to get
               							 'FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET'
               *knock_window_angle_table - This is a pointer to the table of 
               													angles. The angles in the table should 
               													be specified from 0 to 71999, 
               													where 71999 represents 719.99 degrees.
******************************************************************************/       							
int32_t fs_etpu_knock_window_init ( uint8_t channel,
                             uint8_t priority,
                             uint8_t number_of_windows,
                             uint8_t edge_polarity,
                             uint8_t edge_interrupt,                            
                              uint8_t cam_chan,
                              uint32_t *knock_window_angle_table
                            )
{
    uint32_t *knock_pba, *cam_pba;	 /* parameter base address for KNOCK, 
    																														CAM channel */
    uint8_t i;
    uint24_t *Table_Start_Ptr;
    uint32_t *cam_tcr2_count_per_engine_cycle_ptr;
    uint32_t *cam_tcr2_count_per_engine_cycle_ptr_host;
    uint8_t channel_reuse = 0;
    uint32_t *pba_angle_table, *ptr;        /*parameter base address for 
    																														angles table */
    int32_t error_code = 0; /* returned value from etpu API functions */
                                 
	  /* Disable KNOCK channel to assign function safely */
	  fs_etpu_disable( channel );

    if (eTPU->CHAN[channel].CR.B.CPBA == 0 )
    {
    	/* get parameter RAM for
    	number of parameters passed from eTPU C code */
    	knock_pba = fs_etpu_malloc(FS_ETPU_KNOCK_WINDOW_NUM_PARMS );
			
			if (knock_pba == 0)
    	{
    		return (FS_ETPU_ERROR_MALLOC);
    	}
    }
    else /*set pba to what is in the CR register*/
    {
    	knock_pba=fs_etpu_data_ram(channel);
      channel_reuse = 1;
    }
    cam_pba=fs_etpu_data_ram(cam_chan);
    
	  eTPU->CHAN[channel].CR.B.CPBA = (((uint32_t)knock_pba 
	  																		-  fs_etpu_data_ram_start)>>3);

    /* Parameter range chacking */
    if (number_of_windows > 8)
    {
    error_code = FS_ETPU_ERROR_VALUE;
    number_of_windows = 8;
    }
    
	  /* write parameters to data memory for KNOCK */

    Table_Start_Ptr =
        knock_pba + (FS_ETPU_KNOCK_WINDOW_NUM_PARMS) / 4 - \
        (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4));

    cam_tcr2_count_per_engine_cycle_ptr = cam_pba \
       -  (uint32_t) ((uint32_t *) ((fs_etpu_data_ram_start) / 4)) \
                   + (FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET >>2);  

    cam_tcr2_count_per_engine_cycle_ptr_host = cam_pba \
                   + (FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET >>2);                      

   *(knock_pba + ((FS_ETPU_KNOCK_TABLE_START_PTR_OFFSET - 1)>>2)) \
   																						= (uint24_t)Table_Start_Ptr;
   *(knock_pba + ((FS_ETPU_KNOCK_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) \
   														= (uint24_t)cam_tcr2_count_per_engine_cycle_ptr;  
   														
   *((uint8_t *)knock_pba + FS_ETPU_KNOCK_NUMBER_OF_WINDOWS_OFFSET) = number_of_windows;    

   if (channel_reuse == 0)
      {
          /* get parameter RAM
             for the Angles Table */
        pba_angle_table = fs_etpu_malloc (((number_of_windows*2)+2) << 3 );


         if (pba_angle_table == 0)
         {
             return (FS_ETPU_ERROR_MALLOC);
         }
      }
   else      /* re-using parameter RAM which has already been allocated */
      {
          pba_angle_table = (knock_pba + (FS_ETPU_KNOCK_WINDOW_NUM_PARMS) / 4);
      }

   /* populate the angle table in the PRAM */  
   for (i=0; i<(number_of_windows<<1); i++)
   {
   *(pba_angle_table + i) = ((long long)knock_window_angle_table[i]* 
   								*cam_tcr2_count_per_engine_cycle_ptr_host)/(long long)72000;
   }

	 /* write FM (function mode) bits */
	 eTPU->CHAN[channel].SCR.R = edge_polarity + edge_interrupt;
	
   /*write hsr*/
   eTPU->CHAN[channel].HSRR.R = FS_ETPU_KNOCK_INIT;
			
   /* write channel configuration registers */
	 eTPU->CHAN[channel].CR.R =
	      (priority << 28) + 
				(FS_ETPU_KNOCK_WINDOW_TABLE_SELECT << 24) +
	      (FS_ETPU_KNOCK_WINDOW_FUNCTION_NUMBER << 16) +
	      (((uint32_t)knock_pba -  fs_etpu_data_ram_start)>>3);
	
	return(error_code);
}

/******************************************************************************
FUNCTION     : fs_etpu_knock_window_update
PURPOSE      : To update an eTPU channel's KNOCK window parameters.
INPUTS NOTES : This function has the following parameters:
               channel - The knock window channel number. For products with 
               				a single eTPU, this parameter should be assigned a 
               				value of 0-31. For devices with two eTPUs, this 
               				parameter should be assigned a value of 0-31 for eTPU_A 
               				and 64-95 for eTPU_B.
               cam_channel - CAM channel number used to get
               							 'FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET' 
               update_window_number - number of knock window to update.
               										Range is 1 to 8.
               angle_open_new - Value of new open angle for window 
               						to be updated. Range is 0 to 71999. 
               						71999 represents 719.99 degress
               angle_width_new - Value of new angle width for window
               									 to be updated. Range is 0 to 71999. 
               									 71999 represents 719.99 degress
******************************************************************************/
int32_t fs_etpu_knock_window_update ( uint8_t channel,
                                      uint8_t cam_channel, 
                                      uint8_t update_window_number, 
                                      uint32_t angle_open_new, 
                                      uint32_t angle_width_new )
{
	uint32_t *pba_angle_table, *knock_pba, *cam_pba; 
	uint32_t *cam_tcr2_count_per_engine_cycle_ptr_host, ctbase_parm0;
  uint8_t number_of_windows;
  int32_t error_code = 0; /* returned value from etpu API functions */
  
  /* Parameter range chacking */
  if (update_window_number > 8)
  {
  error_code = FS_ETPU_ERROR_VALUE;
  update_window_number = 8;
  }
  if (angle_open_new > 72000)
  {
  error_code = FS_ETPU_ERROR_VALUE;
  angle_open_new = 72000;
  }
  if (angle_width_new > 72000)
  {
  error_code = FS_ETPU_ERROR_VALUE;
  angle_width_new = 72000;
  }
 	
	/* Initial Calculations */
	knock_pba = fs_etpu_data_ram(channel);
	pba_angle_table = (knock_pba + (FS_ETPU_KNOCK_WINDOW_NUM_PARMS) / 4);
  cam_pba = fs_etpu_data_ram(cam_channel);
  cam_tcr2_count_per_engine_cycle_ptr_host = cam_pba + \
                        (FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET >>2);
	number_of_windows = fs_etpu_get_chan_local_8(channel, \
                        FS_ETPU_KNOCK_NUMBER_OF_WINDOWS_OFFSET);

	/* Copy new paramters to temporary area for CDC */
	*(pba_angle_table + (number_of_windows*2)) = ((long long)angle_open_new* \
	                *cam_tcr2_count_per_engine_cycle_ptr_host)/(long long)72000;
	*(pba_angle_table + (number_of_windows*2)+1) = ((long long)angle_width_new* \
	                *cam_tcr2_count_per_engine_cycle_ptr_host)/(long long)72000;
	
	/* Initialization of CDC */
	eTPU->CDCR.B.PBASE = (uint32_t)((pba_angle_table + (number_of_windows*2) \
	                                              - fs_etpu_data_ram_start))>>3; 

  ctbase_parm0 =	(uint32_t)(((uint32_t)pba_angle_table \
                                       - (uint32_t)fs_etpu_data_ram_start))>>2;
  
	eTPU->CDCR.B.CTBASE = (ctbase_parm0>>7) & 0x1F;                
  
	eTPU->CDCR.B.PARAM0 = update_window_number*2 + (ctbase_parm0 & 0x7F);
	eTPU->CDCR.B.PARAM1 = (update_window_number*2)+1 + (ctbase_parm0 & 0x7F);
	
	eTPU->CDCR.B.WR = 0x1; /* read operation */
	
	/* disable interrupts if enabled */
	if (eTPU->CHAN[channel].CR.B.CIE == 1)
		{
	fs_etpu_interrupt_disable(channel);
	/* write STS */
	eTPU->CDCR.B.STS = 1;
	/* re-enable interrupts */
	fs_etpu_interrupt_enable(channel);
	  }
	 else
	  {
	/* write STS */
	eTPU->CDCR.B.STS = 1;
    }
	  
	return(error_code);
}
#if __CWCC__
#pragma pop
#endif