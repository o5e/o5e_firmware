/*******************************************************************************
* FILE NAME: $RCSfile: etpu_fuel.c,v $
*                                     COPYRIGHT (c) Freescale Semiconductor 2008
*                                               All Rights Reserved
* DESCRIPTION:
* This file contains the eTPU FUEL function API.
*===============================================================================
* ORIGINAL AUTHOR: Michal Princ (prnm001)                                
* $Log: etpu_fuel.c,v $
* Revision 1.7  2009/02/04 08:00:15  prnm001
* Make TCR count calculations less likely to overflow
*
* Revision 1.6  2009/01/15 12:22:29  prnm001
* description of priority function parameter changed
*
* Revision 1.5  2009/01/14 14:07:27  prnm001
* CAM_chan -> cam_chan
* ms -> us
* function descriptions modified
*
* Revision 1.4  2008/11/27 07:25:20  prnm001
* void in fs_etpu_fuel_get_sum_of_injection_time
*
* Revision 1.3  2008/11/10 15:26:32  prnm001
* Enable to set injection time of all FUEL channels in the fuel init API functions
*
* Revision 1.2  2008/09/29 13:01:44  prnm001
* before testing
*
* Revision 1.1  2008/06/23 08:16:04  prnm001
* Initial version
*
*
*******************************************************************************/

#include "etpu_util.h"      /* Utility routines for working with the eTPU */
#include "etpu_fuel.h"      /* eTPU FUEL API defines */

/*******************************************************************************
* Global variables
*******************************************************************************/
extern uint32_t fs_etpu_data_ram_start;
extern uint32_t fs_etpu_data_ram_ext;
extern uint32_t etpu_a_tcr1_freq;
extern uint32_t etpu_b_tcr1_freq;

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_init_3cylinders
*
* PURPOSE: Initialize 3 eTPU channels to generate the fuel injection signal.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_1          - The number of the channel which generates the fuel 
*                        signal for the cylinder #1.
*   channel_2          - The number of the channel which generates the fuel 
*                        signal for the cylinder #2.
*   channel_3          - The number of the channel which generates the fuel 
*                        signal for the cylinder #3.
*   cam_chan           - This is the channel number the CAM function is 
*                        assigned to. The FUEL function reads the 
*                        TCR2_Counts_Per_Engine_Cycle value from CAM.
*   priority           - This is the priority to assign to all FUEL channels. 
*                        This parameter should be assigned a value of:
*                        FS_ETPU_PRIORITY_HIGH,
*                        FS_ETPU_PRIORITY_MIDDLE,
*                        FS_ETPU_PRIORITY_LOW or
*                        FS_ETPU_PRIORITY_DISABLED.
*   polarity           - This is the polarity of the output signal. It should be
*                        assigned one of these values:
*                        FS_ETPU_FUEL_PULSE_HIGH or
*                        FS_ETPU_FUEL_PULSE_LOW.
*   cylinder_offset_angle_1 - Offset angle for the engine cylinder #1.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_2 - Offset angle for the engine cylinder #2.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_3 - Offset angle for the engine cylinder #3.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   drop_dead_angle    - This is the closing angle of the intake valve. 
*                        No additional fuel can be put into the cylinder after 
*                        this angle. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the drop 
*                        dead angle for all the fuel signals.
*   injection_normal_end_angle - This is the angle where the injection should be
*                        finished. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the normal 
*                        end angle for all the fuel signals.
*   recalculation_offset_angle - (Injection_Start_Angle-Recalculation_Offset_Angle)
*                        defines the angle where the Re-calc thread is 
*                        scheduled. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the 
*                        recalculation angle for all the fuel signals.
*   injection_time_us_1- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #1. If it is 
*                        not necessary to generate injection pulses immediately 
*                        after the FUEL INIT, set these parameters to zero and 
*                        later use the fs_etpu_fuel_set_injection_time function 
*                        to change the injection time for all individual FUEL 
*                        channels.
*                        The same applies for all FUEL channels.
*   injection_time_us_2- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #2.
*   injection_time_us_3- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #3. 
*   compensation_time_us - Each generated injection pulse is prolonged by this 
*                        time to compensate for the opening and closing time of 
*                        the valve. The time is measured in microseconds. This 
*                        value is influenced by valve parameters, temperature, 
*                        battery voltage, etc. This parameter is global and sets
*                        the compensation time for all the fuel signals.
*   minimum_injection_time_us - This is the required minimum injection pulse 
*                        width, in microseconds. This parameter is global and 
*                        sets the minimum injection time for all the fuel signals.
*   minimum_off_time_us - This is the minimum time between two injection pulses,
*                        in microseconds. This parameter is global and sets the 
*                        normal off time for all the fuel signals.
*
* RETURNS NOTES:Error code if channel could not be initialized. Error code that
*               can be returned are: FS_ETPU_ERROR_MALLOC , FS_ETPU_ERROR_VALUE.
* WARNING      :This function does not configure the pin only the eTPU. In a
*               system a pin may need to be configured to select the eTPU.
*
*******************************************************************************/
int32_t fs_etpu_fuel_init_3cylinders(uint8_t  channel_1,
                                     uint8_t  channel_2,
                                     uint8_t  channel_3,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t cylinder_offset_angle_2,
                                     uint24_t cylinder_offset_angle_3,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t injection_time_us_2,
                                     uint24_t injection_time_us_3,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us)
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba_1, *pba_2, *pba_3;  /* channel parameter base addresses */
  uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
  uint24_t TCR2_Counts_Per_Engine_Cycle;
  uint32_t etpu_tcr1_freq;

  if((cylinder_offset_angle_1>72000)||
     (cylinder_offset_angle_2>72000)||
     (cylinder_offset_angle_3>72000)||
     (drop_dead_angle>72000)||
     (injection_normal_end_angle>72000)||
     (recalculation_offset_angle>72000))
  {
     return(FS_ETPU_ERROR_VALUE);
  }

  if (channel_1<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;

  /* Disable Fuel channels to assign function safely */
  fs_etpu_disable(channel_1);
  fs_etpu_disable(channel_2);
  fs_etpu_disable(channel_3);

  /* Allocate parameter RAM for channels */
  if (eTPU->CHAN[channel_1].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_1=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_1 = fs_etpu_data_ram(channel_1);
  }

  if (eTPU->CHAN[channel_2].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_2=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_2 = fs_etpu_data_ram(channel_2);
  }

  if (eTPU->CHAN[channel_3].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_3=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_3 = fs_etpu_data_ram(channel_3);
  }

  /* write Fuel function global parameters */
  TCR2_Counts_Per_Engine_Cycle = fs_etpu_get_chan_local_24(cam_chan,  FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_DROP_DEAD_ANGLE_OFFSET            - 1)>>2)) = ((long long)drop_dead_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_INJECTION_NORMAL_END_ANGLE_OFFSET - 1)>>2)) = ((long long)injection_normal_end_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_RECALCULATION_OFFSET_ANGLE_OFFSET - 1)>>2)) = ((long long)recalculation_offset_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_COMPENSATION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * compensation_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_MINIMUM_INJECTION_TIME_OFFSET     - 1)>>2)) = ((etpu_tcr1_freq/10000) * minimum_injection_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_MINIMUM_OFF_TIME_OFFSET           - 1)>>2)) = ((etpu_tcr1_freq/10000) * minimum_off_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_SUM_OF_INJECTION_TIME_OFFSET      - 1)>>2)) = 0;

  /* write parameters to data memory for Fuel */
  *(pba_1 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_1 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_1 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_1) / 100;
  *(pba_1 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_1 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_2 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_2 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_2 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_2) / 100;
  *(pba_2 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_2 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_3 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_3 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_3 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_3) / 100;
  *(pba_3 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_3 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;

  TCR2_Counts_Per_Engine_Cycle_addr = (uint32_t)((eTPU->CHAN[cam_chan].CR.B.CPBA << 3) + FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  *(pba_1 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_2 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_3 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;


  /* write channel configuration registers */
  eTPU->CHAN[channel_1].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_1 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_2].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_2 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_3].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_3 - fs_etpu_data_ram_start)>>3);

  /* write FM (function mode) bits */
  eTPU->CHAN[channel_1].SCR.R = polarity;
  eTPU->CHAN[channel_2].SCR.R = polarity;
  eTPU->CHAN[channel_3].SCR.R = polarity;

  /* write hsr to start channel running */
  eTPU->CHAN[channel_1].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_2].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_3].HSRR.R = FS_ETPU_FUEL_INIT;

  return(error_code);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_init_4cylinders
*
* PURPOSE: Initialize 4 eTPU channels to generate the fuel injection signal.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_1          - The number of the channel which generates the fuel 
*                        signal for the cylinder #1.
*   channel_2          - The number of the channel which generates the fuel 
*                        signal for the cylinder #2.
*   channel_3          - The number of the channel which generates the fuel 
*                        signal for the cylinder #3.
*   channel_4          - The number of the channel which generates the fuel 
*                        signal for the cylinder #4.
*   cam_chan           - This is the channel number the CAM function is 
*                        assigned to. The FUEL function reads the 
*                        TCR2_Counts_Per_Engine_Cycle value from CAM.
*   priority           - This is the priority to assign to all FUEL channels. 
*                        This parameter should be assigned a value of:
*                        FS_ETPU_PRIORITY_HIGH,
*                        FS_ETPU_PRIORITY_MIDDLE,
*                        FS_ETPU_PRIORITY_LOW or
*                        FS_ETPU_PRIORITY_DISABLED.
*   polarity           - This is the polarity of the output signal. It should be
*                        assigned one of these values:
*                        FS_ETPU_FUEL_PULSE_HIGH or
*                        FS_ETPU_FUEL_PULSE_LOW.
*   cylinder_offset_angle_1 - Offset angle for the engine cylinder #1.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_2 - Offset angle for the engine cylinder #2.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_3 - Offset angle for the engine cylinder #3.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_4 - Offset angle for the engine cylinder #4.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   drop_dead_angle    - This is the closing angle of the intake valve. 
*                        No additional fuel can be put into the cylinder after 
*                        this angle. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the drop 
*                        dead angle for all the fuel signals.
*   injection_normal_end_angle - This is the angle where the injection should be
*                        finished. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the normal 
*                        end angle for all the fuel signals.
*   recalculation_offset_angle - (Injection_Start_Angle-Recalculation_Offset_Angle)
*                        defines the angle where the Re-calc thread is 
*                        scheduled. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the 
*                        recalculation angle for all the fuel signals.
*   injection_time_us_1- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #1. If it is 
*                        not necessary to generate injection pulses immediately 
*                        after the FUEL INIT, set these parameters to zero and 
*                        later use the fs_etpu_fuel_set_injection_time function 
*                        to change the injection time for all individual FUEL 
*                        channels.
*                        The same applies for all FUEL channels.
*   injection_time_us_2- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #2. 
*   injection_time_us_3- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #3. 
*   injection_time_us_4- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #4. 
*   compensation_time_us - Each generated injection pulse is prolonged by this 
*                        time to compensate for the opening and closing time of 
*                        the valve. The time is measured in microseconds. This 
*                        value is influenced by valve parameters, temperature, 
*                        battery voltage, etc. This parameter is global and sets
*                        the compensation time for all the fuel signals.
*   minimum_injection_time_us - This is the required minimum injection pulse 
*                        width, in microseconds. This parameter is global and 
*                        sets the minimum injection time for all the fuel signals.
*   minimum_off_time_us - This is the minimum time between two injection pulses,
*                        in microseconds. This parameter is global and sets the 
*                        normal off time for all the fuel signals.
*
* RETURNS NOTES:Error code if channel could not be initialized. Error code that
*               can be returned are: FS_ETPU_ERROR_MALLOC , FS_ETPU_ERROR_VALUE.
* WARNING      :This function does not configure the pin only the eTPU. In a
*               system a pin may need to be configured to select the eTPU.
*
*******************************************************************************/
int32_t fs_etpu_fuel_init_4cylinders(uint8_t  channel_1,
                                     uint8_t  channel_2,
                                     uint8_t  channel_3,
                                     uint8_t  channel_4,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t cylinder_offset_angle_2,
                                     uint24_t cylinder_offset_angle_3,
                                     uint24_t cylinder_offset_angle_4,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t injection_time_us_2,
                                     uint24_t injection_time_us_3,
                                     uint24_t injection_time_us_4,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us)
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba_1, *pba_2, *pba_3, *pba_4;  /* channel parameter base addresses */
  uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
  uint24_t TCR2_Counts_Per_Engine_Cycle;
  uint32_t etpu_tcr1_freq;

  if((cylinder_offset_angle_1>72000)||
     (cylinder_offset_angle_2>72000)||
     (cylinder_offset_angle_3>72000)||
     (cylinder_offset_angle_4>72000)||
     (drop_dead_angle>72000)||
     (injection_normal_end_angle>72000)||
     (recalculation_offset_angle>72000))
  {
     return(FS_ETPU_ERROR_VALUE);
  }

  if (channel_1<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;

  /* Disable Fuel channels to assign function safely */
  fs_etpu_disable(channel_1);
  fs_etpu_disable(channel_2);
  fs_etpu_disable(channel_3);
  fs_etpu_disable(channel_4);

  /* Allocate parameter RAM for channels */
  if (eTPU->CHAN[channel_1].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_1=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_1 = fs_etpu_data_ram(channel_1);
  }

  if (eTPU->CHAN[channel_2].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_2=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_2 = fs_etpu_data_ram(channel_2);
  }

  if (eTPU->CHAN[channel_3].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_3=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_3 = fs_etpu_data_ram(channel_3);
  }

  if (eTPU->CHAN[channel_4].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_4=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_4 = fs_etpu_data_ram(channel_4);
  }

  /* write Fuel function global parameters */
  TCR2_Counts_Per_Engine_Cycle = fs_etpu_get_chan_local_24(cam_chan,  FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_DROP_DEAD_ANGLE_OFFSET            - 1)>>2)) = ((long long)drop_dead_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_INJECTION_NORMAL_END_ANGLE_OFFSET - 1)>>2)) = ((long long)injection_normal_end_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_RECALCULATION_OFFSET_ANGLE_OFFSET - 1)>>2)) = ((long long)recalculation_offset_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_COMPENSATION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * compensation_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_MINIMUM_INJECTION_TIME_OFFSET     - 1)>>2)) = ((etpu_tcr1_freq/10000) * minimum_injection_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_MINIMUM_OFF_TIME_OFFSET           - 1)>>2)) = ((etpu_tcr1_freq/10000) * minimum_off_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_SUM_OF_INJECTION_TIME_OFFSET      - 1)>>2)) = 0;

  /* write parameters to data memory for Fuel */
  *(pba_1 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_1 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_1 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_1) / 100;
  *(pba_1 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_1 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_2 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_2 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_2 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_2) / 100;
  *(pba_2 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_2 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_3 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_3 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_3 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_3) / 100;
  *(pba_3 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_3 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_4 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_4 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_4 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_4) / 100;
  *(pba_4 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_4 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;

  TCR2_Counts_Per_Engine_Cycle_addr = (uint32_t)((eTPU->CHAN[cam_chan].CR.B.CPBA << 3) + FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  *(pba_1 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_2 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_3 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_4 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;


  /* write channel configuration registers */
  eTPU->CHAN[channel_1].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_1 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_2].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_2 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_3].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_3 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_4].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_4 - fs_etpu_data_ram_start)>>3);

  /* write FM (function mode) bits */
  eTPU->CHAN[channel_1].SCR.R = polarity;
  eTPU->CHAN[channel_2].SCR.R = polarity;
  eTPU->CHAN[channel_3].SCR.R = polarity;
  eTPU->CHAN[channel_4].SCR.R = polarity;

  /* write hsr to start channel running */
  eTPU->CHAN[channel_1].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_2].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_3].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_4].HSRR.R = FS_ETPU_FUEL_INIT;

  return(error_code);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_init_6cylinders
*
* PURPOSE: Initialize 6 eTPU channels to generate the fuel injection signal.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_1          - The number of the channel which generates the fuel 
*                        signal for the cylinder #1.
*   channel_2          - The number of the channel which generates the fuel 
*                        signal for the cylinder #2.
*   channel_3          - The number of the channel which generates the fuel 
*                        signal for the cylinder #3.
*   channel_4          - The number of the channel which generates the fuel 
*                        signal for the cylinder #4.
*   channel_5          - The number of the channel which generates the fuel 
*                        signal for the cylinder #5.
*   channel_6          - The number of the channel which generates the fuel 
*                        signal for the cylinder #6.
*   cam_chan           - This is the channel number the CAM function is 
*                        assigned to. The FUEL function reads the 
*                        TCR2_Counts_Per_Engine_Cycle value from CAM.
*   priority           - This is the priority to assign to all FUEL channels. 
*                        This parameter should be assigned a value of:
*                        FS_ETPU_PRIORITY_HIGH,
*                        FS_ETPU_PRIORITY_MIDDLE,
*                        FS_ETPU_PRIORITY_LOW or
*                        FS_ETPU_PRIORITY_DISABLED.
*   polarity           - This is the polarity of the output signal. It should be
*                        assigned one of these values:
*                        FS_ETPU_FUEL_PULSE_HIGH or
*                        FS_ETPU_FUEL_PULSE_LOW.
*   cylinder_offset_angle_1 - Offset angle for the engine cylinder #1.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_2 - Offset angle for the engine cylinder #2.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_3 - Offset angle for the engine cylinder #3.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_4 - Offset angle for the engine cylinder #4.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_5 - Offset angle for the engine cylinder #5.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   cylinder_offset_angle_6 - Offset angle for the engine cylinder #6.
*                        Range 0 to 71999. 71999 represents 719.99 degrees.
*   drop_dead_angle    - This is the closing angle of the intake valve. 
*                        No additional fuel can be put into the cylinder after 
*                        this angle. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the drop 
*                        dead angle for all the fuel signals.
*   injection_normal_end_angle - This is the angle where the injection should be
*                        finished. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the normal 
*                        end angle for all the fuel signals.
*   recalculation_offset_angle - (Injection_Start_Angle-Recalculation_Offset_Angle)
*                        defines the angle where the Re-calc thread is 
*                        scheduled. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the 
*                        recalculation angle for all the fuel signals.
*   injection_time_us_1- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #1. If it is 
*                        not necessary to generate injection pulses immediately 
*                        after the FUEL INIT, set these parameters to zero and 
*                        later use the fs_etpu_fuel_set_injection_time function 
*                        to change the injection time for all individual FUEL 
*                        channels.
*                        The same applies for all FUEL channels.
*   injection_time_us_2- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #2. 
*   injection_time_us_3- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #3. 
*   injection_time_us_4- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #4. 
*   injection_time_us_5- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #5. 
*   injection_time_us_6- This is the length of the injection pulse, 
*                        in microseconds, for the engine cylinder #6. 
*   compensation_time_us - Each generated injection pulse is prolonged by this 
*                        time to compensate for the opening and closing time of 
*                        the valve. The time is measured in microseconds. This 
*                        value is influenced by valve parameters, temperature, 
*                        battery voltage, etc. This parameter is global and sets
*                        the compensation time for all the fuel signals.
*   minimum_injection_time_us - This is the required minimum injection pulse 
*                        width, in microseconds. This parameter is global and 
*                        sets the minimum injection time for all the fuel signals.
*   minimum_off_time_us - This is the minimum time between two injection pulses,
*                        in microseconds. This parameter is global and sets the 
*                        normal off time for all the fuel signals.
*
* RETURNS NOTES:Error code if channel could not be initialized. Error code that
*               can be returned are: FS_ETPU_ERROR_MALLOC , FS_ETPU_ERROR_VALUE.
* WARNING      :This function does not configure the pin only the eTPU. In a
*               system a pin may need to be configured to select the eTPU.
*
*******************************************************************************/
int32_t fs_etpu_fuel_init_6cylinders(uint8_t  channel_1,
                                     uint8_t  channel_2,
                                     uint8_t  channel_3,
                                     uint8_t  channel_4,
                                     uint8_t  channel_5,
                                     uint8_t  channel_6,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t cylinder_offset_angle_2,
                                     uint24_t cylinder_offset_angle_3,
                                     uint24_t cylinder_offset_angle_4,
                                     uint24_t cylinder_offset_angle_5,
                                     uint24_t cylinder_offset_angle_6,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t injection_time_us_2,
                                     uint24_t injection_time_us_3,
                                     uint24_t injection_time_us_4,
                                     uint24_t injection_time_us_5,
                                     uint24_t injection_time_us_6,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us)
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba_1, *pba_2, *pba_3, *pba_4, *pba_5, *pba_6;  /* channel parameter base addresses */
  uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
  uint24_t TCR2_Counts_Per_Engine_Cycle;
  uint32_t etpu_tcr1_freq;

  if((cylinder_offset_angle_1>72000)||
     (cylinder_offset_angle_2>72000)||
     (cylinder_offset_angle_3>72000)||
     (cylinder_offset_angle_4>72000)||
     (cylinder_offset_angle_5>72000)||
     (cylinder_offset_angle_6>72000)||
     (drop_dead_angle>72000)||
     (injection_normal_end_angle>72000)||
     (recalculation_offset_angle>72000))
  {
     return(FS_ETPU_ERROR_VALUE);
  }

  if (channel_1<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;

  /* Disable Fuel channels to assign function safely */
  fs_etpu_disable(channel_1);
  fs_etpu_disable(channel_2);
  fs_etpu_disable(channel_3);
  fs_etpu_disable(channel_4);
  fs_etpu_disable(channel_5);
  fs_etpu_disable(channel_6);

  /* Allocate parameter RAM for channels */
  if (eTPU->CHAN[channel_1].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_1=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_1 = fs_etpu_data_ram(channel_1);
  }

  if (eTPU->CHAN[channel_2].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_2=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_2 = fs_etpu_data_ram(channel_2);
  }

  if (eTPU->CHAN[channel_3].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_3=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_3 = fs_etpu_data_ram(channel_3);
  }

  if (eTPU->CHAN[channel_4].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_4=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_4 = fs_etpu_data_ram(channel_4);
  }

  if (eTPU->CHAN[channel_5].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_5=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_5 = fs_etpu_data_ram(channel_5);
  }

  if (eTPU->CHAN[channel_6].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    if ((pba_6=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)
    {
       return(FS_ETPU_ERROR_MALLOC);
    }
  }
  else /* set pba to what is in the CR register */
  {
    pba_6 = fs_etpu_data_ram(channel_6);
  }

  /* write Fuel function global parameters */
  TCR2_Counts_Per_Engine_Cycle = fs_etpu_get_chan_local_24(cam_chan,  FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_DROP_DEAD_ANGLE_OFFSET            - 1)>>2)) = ((long long)drop_dead_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_INJECTION_NORMAL_END_ANGLE_OFFSET - 1)>>2)) = ((long long)injection_normal_end_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_RECALCULATION_OFFSET_ANGLE_OFFSET - 1)>>2)) = ((long long)recalculation_offset_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_COMPENSATION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * compensation_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_MINIMUM_INJECTION_TIME_OFFSET     - 1)>>2)) = ((etpu_tcr1_freq/10000) * minimum_injection_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_MINIMUM_OFF_TIME_OFFSET           - 1)>>2)) = ((etpu_tcr1_freq/10000) * minimum_off_time_us) / 100;
  *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_FUEL_SUM_OF_INJECTION_TIME_OFFSET      - 1)>>2)) = 0;

  /* write parameters to data memory for Fuel */
  *(pba_1 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_1 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_1 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_1) / 100;
  *(pba_1 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_1 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_2 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_2 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_2 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_2) / 100;
  *(pba_2 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_2 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_3 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_3 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_3 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_3) / 100;
  *(pba_3 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_3 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_4 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_4 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_4 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_4) / 100;
  *(pba_4 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_4 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_5 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_5 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_5 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_5) / 100;
  *(pba_5 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_5 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;
  *(pba_6 + ((FS_ETPU_FUEL_CYLINDER_OFFSET_ANGLE_OFFSET   - 1)>>2)) = ((long long)cylinder_offset_angle_6 * TCR2_Counts_Per_Engine_Cycle)/(long long)72000;
  *(pba_6 + ((FS_ETPU_FUEL_INJECTION_TIME_OFFSET          - 1)>>2)) = ((etpu_tcr1_freq/10000) * injection_time_us_6) / 100;
  *(pba_6 + ((FS_ETPU_FUEL_REAL_INJECTION_TIME_OFFSET     - 1)>>2)) = 0;
  *(pba_6 + ((FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET - 1)>>2)) = 0;

  TCR2_Counts_Per_Engine_Cycle_addr = (uint32_t)((eTPU->CHAN[cam_chan].CR.B.CPBA << 3) + FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  *(pba_1 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_2 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_3 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_4 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_5 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;
  *(pba_6 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;


  /* write channel configuration registers */
  eTPU->CHAN[channel_1].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_1 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_2].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_2 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_3].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_3 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_4].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_4 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_5].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_5 - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_6].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_6 - fs_etpu_data_ram_start)>>3);

  /* write FM (function mode) bits */
  eTPU->CHAN[channel_1].SCR.R = polarity;
  eTPU->CHAN[channel_2].SCR.R = polarity;
  eTPU->CHAN[channel_3].SCR.R = polarity;
  eTPU->CHAN[channel_4].SCR.R = polarity;
  eTPU->CHAN[channel_5].SCR.R = polarity;
  eTPU->CHAN[channel_6].SCR.R = polarity;

  /* write hsr to start channel running */
  eTPU->CHAN[channel_1].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_2].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_3].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_4].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_5].HSRR.R = FS_ETPU_FUEL_INIT;
  eTPU->CHAN[channel_6].HSRR.R = FS_ETPU_FUEL_INIT;

  return(error_code);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_set_injection_time
*
* PURPOSE: Change the injection time.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*   injection_time_us  - The length of the injection pulse in microseconds.
*
* RETURNS NOTES: This function returns 0 if the injection time update was 
*                successfull. If the Fuel channel has any pending HSRs, this 
*                function should be called again later. In this case, a sum 
*                of pending HSR numbers is returned.
*******************************************************************************/
int32_t fs_etpu_fuel_set_injection_time(uint8_t  channel,
                                        uint24_t injection_time_us)
{
  uint32_t etpu_tcr1_freq;
  uint32_t hsrr;

  if (channel<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;
  
  fs_etpu_set_chan_local_24(channel,  
	                          FS_ETPU_FUEL_INJECTION_TIME_OFFSET,
	                          ((etpu_tcr1_freq/10000) * injection_time_us)/ 100) ;

  /* write HSR; if the Fuel channel has any pending HSRs do not write
     new HSRs but return sum of pending HSR numbers */
  hsrr = eTPU->CHAN[channel].HSRR.R;
  if (hsrr != 0) return ((int32_t)hsrr);
  
  eTPU->CHAN[channel].HSRR.R = FS_ETPU_FUEL_INJECTION_TIME_CHANGE;

  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_set_drop_dead_angle
*
* PURPOSE: Change the drop dead angle.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*   drop_dead_angle    - This is the closing angle of the intake valve. 
*                        No additional fuel can be put into the cylinder after 
*                        this angle. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the drop 
*                        dead angle for all the fuel signals.
*
* RETURNS NOTES: Error code that can be returned is: FS_ETPU_ERROR_VALUE.
*
*******************************************************************************/
int32_t fs_etpu_fuel_set_drop_dead_angle(uint8_t  channel,
                                         uint24_t drop_dead_angle)
{
  uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
  uint24_t TCR2_Counts_Per_Engine_Cycle;

  if(drop_dead_angle>72000)
  {
     return(FS_ETPU_ERROR_VALUE);
  }

  TCR2_Counts_Per_Engine_Cycle_addr = 
    fs_etpu_get_chan_local_24(channel, FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  
  TCR2_Counts_Per_Engine_Cycle = *((uint32_t*)(fs_etpu_data_ram_start + TCR2_Counts_Per_Engine_Cycle_addr-1));
  TCR2_Counts_Per_Engine_Cycle &= 0xffffff;
  
  fs_etpu_set_global_24(FS_ETPU_FUEL_DROP_DEAD_ANGLE_OFFSET,  
						((long long)drop_dead_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000);
  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_set_normal_end_angle
*
* PURPOSE: Change the normal end angle.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*   normal_end_angle   - This is the angle where the injection should be
*                        finished. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the normal 
*                        end angle for all the fuel signals.
*
* RETURNS NOTES: Error code that can be returned is: FS_ETPU_ERROR_VALUE.
*
*******************************************************************************/
int32_t fs_etpu_fuel_set_normal_end_angle(uint8_t  channel,
                                          uint24_t normal_end_angle)
{
  uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
  uint24_t TCR2_Counts_Per_Engine_Cycle;

  if(normal_end_angle>72000)
  {
     return(FS_ETPU_ERROR_VALUE);
  }

  TCR2_Counts_Per_Engine_Cycle_addr = 
    fs_etpu_get_chan_local_24(channel, FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  
  TCR2_Counts_Per_Engine_Cycle = *((uint32_t*)(fs_etpu_data_ram_start + TCR2_Counts_Per_Engine_Cycle_addr-1));
  TCR2_Counts_Per_Engine_Cycle &= 0xffffff;
  
  fs_etpu_set_global_24(FS_ETPU_FUEL_INJECTION_NORMAL_END_ANGLE_OFFSET,  
						((long long)normal_end_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000);
  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_set_recalc_offset_angle
*
* PURPOSE: Change the recalculation offset angle.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*   recalculation_offset_angle - (Injection_Start_Angle-Recalculation_Offset_Angle)
*                        defines the angle where the Re-calc thread is 
*                        scheduled. Range 0 to 71999. 71999 represents 719.99 
*                        degrees. This parameter is global and sets the 
*                        recalculation angle for all the fuel signals.
*
* RETURNS NOTES: Error code that can be returned is: FS_ETPU_ERROR_VALUE.
*
*******************************************************************************/
int32_t fs_etpu_fuel_set_recalc_offset_angle(uint8_t  channel,
                                             uint24_t recalc_offset_angle)
{
  uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
  uint24_t TCR2_Counts_Per_Engine_Cycle;

  if(recalc_offset_angle>72000)
  {
     return(FS_ETPU_ERROR_VALUE);
  }

  TCR2_Counts_Per_Engine_Cycle_addr = 
    fs_etpu_get_chan_local_24(channel, FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  
  TCR2_Counts_Per_Engine_Cycle = *((uint32_t*)(fs_etpu_data_ram_start + TCR2_Counts_Per_Engine_Cycle_addr-1));
  TCR2_Counts_Per_Engine_Cycle &= 0xffffff;
  
  fs_etpu_set_global_24(FS_ETPU_FUEL_RECALCULATION_OFFSET_ANGLE_OFFSET,  
						((long long)recalc_offset_angle * TCR2_Counts_Per_Engine_Cycle)/(long long)72000);
  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_set_compensation_time
*
* PURPOSE: Change the compensation time.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*   compensation_time_us - Compensates for the opening and closing time
*                        of the valve; in us.
*
*******************************************************************************/
int32_t fs_etpu_fuel_set_compensation_time(uint8_t  channel,
                                           uint24_t compensation_time_us)
{
  uint32_t etpu_tcr1_freq;

  if (channel<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;
  
  fs_etpu_set_global_24(FS_ETPU_FUEL_COMPENSATION_TIME_OFFSET,  
						((etpu_tcr1_freq/10000) * compensation_time_us) / 100);
  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_set_minimum_injection_time
*
* PURPOSE: Change the minimum injection time.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*   minimum_injection_time_us - This is the required minimum injection pulse 
*                        width, in microseconds. This parameter is global and 
*                        sets the minimum injection time for all the fuel signals.
*
*******************************************************************************/
int32_t fs_etpu_fuel_set_minimum_injection_time(uint8_t  channel,
                                                uint24_t minimum_injection_time_us)
{
  uint32_t etpu_tcr1_freq;

  if (channel<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;
  
  fs_etpu_set_global_24(FS_ETPU_FUEL_MINIMUM_INJECTION_TIME_OFFSET,  
						((etpu_tcr1_freq/10000) * minimum_injection_time_us) / 100);
  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_set_minimum_off_time
*
* PURPOSE: Change the minimum off time.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*   minimum_off_time_us - This is the minimum time between two injection pulses,
*                        in microseconds. This parameter is global and sets the 
*                        normal off time for all the fuel signals.
*
*******************************************************************************/
int32_t fs_etpu_fuel_set_minimum_off_time(uint8_t  channel,
                                          uint24_t minimum_off_time_us)
{
  uint32_t etpu_tcr1_freq;

  if (channel<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;
  
  fs_etpu_set_global_24(FS_ETPU_FUEL_MINIMUM_OFF_TIME_OFFSET,  
						((etpu_tcr1_freq/10000) * minimum_off_time_us) / 100);
  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_switch_off
*
* PURPOSE: This function switches the injection pulse generation off. Switching 
*          off should not be done by setting the Injection Time = 0, as this 
*          would shorten an injection pulse which is currently in progress, 
*          resulting in an incorrect pulse. Instead this function should be 
*          used. In this case the CPU can switch off at any time, but a pulse 
*          which has already been started will be correctly finished. 
*          It is not possible to switch off all initialized FUEL channels by 
*          calling this function; instead the user has to call this function 
*          several times based on the number of initialized FUEL channels.
*                
* INPUTS NOTES: This function has the following parameter:
*   channel          - The number of the FUEL channel which needs to be stopped.
*
*******************************************************************************/
int32_t fs_etpu_fuel_switch_off(uint8_t channel)
{
  /* write FM (function mode) bits */
  eTPU->CHAN[channel].SCR.R |= (FS_ETPU_FUEL_FM1_PULSE_GENERATION_DISABLED<<1);

  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_switch_on
*
* PURPOSE: This function switches the injection pulse generation on. This 
*          function is used once the injections were suspended and the CPU 
*          requests to turn them back on again.
*          It is not possible to switch on all initialized FUEL channels by 
*          calling this function; instead the user has to call this function 
*          several times based on the number of initialized FUEL channels.
*                
* INPUTS NOTES: This function has the following parameter:
*   channel          - The number of the FUEL channel which needs to be started.
*
*******************************************************************************/
int32_t fs_etpu_fuel_switch_on(uint8_t channel)
{
  /* write FM (function mode) bits */
  eTPU->CHAN[channel].SCR.B.FM1 = 0;

  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_get_sum_of_injection_time
*
* PURPOSE: To get the sum of all injection times (total injection time applied 
*          through all initialized FUEL channels) in TCR1 TICKS.
*                
* INPUTS NOTES: This function has no input parameter.
*
* RETURNS NOTES: This function returns the sum of injection time (global variable).
*
*******************************************************************************/
uint24_t fs_etpu_fuel_get_sum_of_injection_time(void)
{
  return(fs_etpu_get_global_24(FS_ETPU_FUEL_SUM_OF_INJECTION_TIME_OFFSET));
}

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_get_CPU_real_injection_time
*
* PURPOSE: To get the real injection time of the defined cylinder (particular 
*          FUEL channel), in the last engine cycle, in TCR1 TICKS.
*                
* INPUTS NOTES: This function has the following parameter:
*   channel            - The number of the channel which generates the fuel 
*                        signal.
*
* RETURNS NOTES: This function returns the real injection time.
*
*******************************************************************************/
uint24_t fs_etpu_fuel_get_CPU_real_injection_time(uint8_t channel)
{
  return(fs_etpu_get_chan_local_24(channel, FS_ETPU_FUEL_CPU_REAL_INJECTION_TIME_OFFSET));
}


/*********************************************************************
 *
 * Copyright:
 *  Freescale Semiconductor, INC. All Rights Reserved.
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