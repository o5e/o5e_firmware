
/*******************************************************************************
*                                     COPYRIGHT (c) Freescale Semiconductor 2008
*                                               All Rights Reserved
* DESCRIPTION:
* This file contains the eTPU FUEL function API.
* JZ - Modified to do a single cylinder
*
*******************************************************************************/

#include "config.h"
#include "etpu_util.h"
#include "variables.h"

/*  eTPU APIs         */
#include "etpu_toothgen.h"
#include "etpu_app_eng_pos.h"
#include "etpu_fuel.h"
#include "etpu_spark.h"
#include "etpu_pwm.h"
#include "eTPU_OPS.h"

extern uint32_t fs_etpu_code_start;
extern uint32_t fs_etpu_data_ram_start;
extern uint32_t fs_etpu_data_ram_end;
extern uint32_t fs_etpu_data_ram_ext;

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif

/*******************************************************************************
* FUNCTION: fs_etpu_fuel_init_cylinder
*
* PURPOSE: Initialize a single eTPU channel to generate the fuel injection signal.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_1          - The number of the channel which generates the fuel 
*                        signal for the cylinder #1.
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
*   compensation_time_us - Each generated injection pulse is prolonged by this 
*                        time to compensate for the opening and closing time of 
*                        the valve. The time is measured in microseconds. This 
*                        value is influenced by valve parameters, temperature, 
*                        battery voltage, etc. This parameter is global and sets
*                        the compensation time for all the fuel signals.
*   minimum_injection_time_us - This is the required minimum injection pulse 
*                        width, in microseconds. This parameter is global and 
*===============================================================================
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
int32_t fs_etpu_fuel_init_cylinder(uint8_t  channel_1,
                                     uint8_t  cam_chan,
                                     uint8_t  priority,
                                     uint8_t  polarity,
                                     uint24_t cylinder_offset_angle_1,
                                     uint24_t drop_dead_angle,
                                     uint24_t injection_normal_end_angle,
                                     uint24_t recalculation_offset_angle,
                                     uint24_t injection_time_us_1,
                                     uint24_t compensation_time_us,
                                     uint24_t minimum_injection_time_us,
                                     uint24_t minimum_off_time_us)
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba_1;  /* channel parameter base addresses */
  uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
  uint24_t TCR2_Counts_Per_Engine_Cycle;

  if((cylinder_offset_angle_1>72000)||     
     (drop_dead_angle>72000)||
     (injection_normal_end_angle>72000)||
     (recalculation_offset_angle>72000))
  {
     return(FS_ETPU_ERROR_VALUE);
  }

  
  /* Disable channel to assign function safely */
  fs_etpu_disable(channel_1);

  /* Allocate parameter RAM for channel if we don't already have it */
  if (eTPU->CHAN[channel_1].CR.B.CPBA == 0 ) {
    if ((pba_1=fs_etpu_malloc(FS_ETPU_FUEL_NUM_PARMS)) == 0)  // get ram
       return(FS_ETPU_ERROR_MALLOC);
  } else /* set pba to what is in the CR register */
    pba_1 = fs_etpu_data_ram(channel_1);

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

  TCR2_Counts_Per_Engine_Cycle_addr = (uint32_t)((eTPU->CHAN[cam_chan].CR.B.CPBA << 3) + FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);
  *(pba_1 + ((FS_ETPU_FUEL_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1)>>2)) = TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF;

  /* write channel configuration registers */
  eTPU->CHAN[channel_1].CR.R = (priority << 28) + (FS_ETPU_FUEL_TABLE_SELECT << 24) +
                               (FS_ETPU_FUEL_FUNCTION_NUMBER << 16) +
                               (((uint32_t)pba_1 - fs_etpu_data_ram_start)>>3);
  /* write FM (function mode) bits */
  eTPU->CHAN[channel_1].SCR.R = polarity;

  /* write hsr to start channel running */
  eTPU->CHAN[channel_1].HSRR.R = FS_ETPU_FUEL_INIT;

  return(error_code);
}

/***************************************************************************
* FILE NAME: etpu_spark.c                     COPYRIGHT (c) FREESCALE 2009 *
*                                                      All Rights Reserved *
*==========================================================================*
* ORIGINAL AUTHOR: Stan Ostrum                LAST UPDATE: 09/01/08 14:27  *
*                                                                          *
* DESCRIPTION: This file contains the source code for the eTPU set2 SPARK  *
*              function API.                                               *
* JZ - Modified to do a single cylinder
*                                                                          *
****************************************************************************
* REVISION HISTORY:
*
* Revision 1.6  2009/01/27 15:29:01  r47354

***************************************************************************/

#include "etpu_util.h"          /* eTPU utility routines */
#include "etpu_spark.h"         /* SPARK API definitions */

/*******************************************************************************
* Global variables
*******************************************************************************/
extern uint32_t fs_etpu_data_ram_start;
extern uint32_t fs_etpu_data_ram_ext;
extern uint32_t etpu_a_tcr1_freq;
extern uint32_t etpu_b_tcr1_freq;

/*******************************************************************************
* FUNCTION: fs_etpu_spark_init_cylinder
*
* PURPOSE:  initialize a eTPU channel to generate SPARK outputs
*
* INPUT PARAMETERS:
*
*   all angle parameters are in units of 1/100 degree with a range of 0 - 71999
*
*   all time parameters are in units of us
*
*   spark_channel_1    - eTPU channel assigned to SPARK for cylinder 1
*   cyl_offset_angle_1 - offset angle for cylinder 1 <0..71999>
*   priority           - selects priority for SPARK
*   polarity           - selects output pin polarity for SPARK
*   min_dwell_time     - minimum dwell time in us
*   max_dwell_time     - maximum dwell time in us
*   multi_on_time      - multi spark pulse active time in us
*   multi_off_time     - multi spark pulse inactive time in us
*   multi_num_pulses   - number of multi spark pulses to generate
*   recalc_offset_angle - angle before estimated start angle at which start angle
*                         is recalculated  <0..71999> 
*   init_dwell_time_1  - dwell time in us for first pulse (used for all channels at init)
*   init_dwell_time_2  - dwell time in us for 2nd pulse (used for all channels at init)
*   init_end_angle_1   - end angle for first pulse (used for all channels at init)  <0..71999> 
*   init_end_angle_2   - end angle for 2nd pulse (used for all channels at init) <0..71999> 
*
* RETURN VALUE:  - 0 if SPARK initialization was successful
*                - FS_ETPU_ERROR_VALUE if any angle parameters are out of range
*                - FS_ETPU_ERROR_MALLOC if a parameter RAM allocation error occurs
*
*******************************************************************************/
int32_t fs_etpu_spark_init_cylinder(  uint8_t  spark_channel_1,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint8_t  priority,
                                        uint8_t  polarity,
                                        uint24_t min_dwell_time,
                                        uint24_t max_dwell_time,
                                        uint24_t multi_on_time,
                                        uint24_t multi_off_time,
                                        uint8_t  multi_num_pulses,
                                        uint24_t recalc_offset_angle,
                                        uint24_t init_dwell_time_1,
                                        uint24_t init_dwell_time_2,
                                        uint24_t init_end_angle_1,
                                        uint24_t init_end_angle_2 )

{
    int32_t  error_code = 0;                    /* return value */
    uint32_t *pba_1;                            /* channel parameter base addresses */
    uint32_t etpu_tcr1_freq_local;              /* TCR1 timebase frequency */
    uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
    uint24_t TCR2_Counts_Per_Engine_Cycle;

    /* disable channels */
    fs_etpu_disable(spark_channel_1);

    if ((cyl_offset_angle_1 > 72000) || (recalc_offset_angle > 72000))
    {
        return (FS_ETPU_ERROR_VALUE);
    }

    /* allocate parameter RAM for channels */
    if (eTPU->CHAN[spark_channel_1].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_1 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_1 = fs_etpu_data_ram(spark_channel_1);
    }

    /* write channel configuration registers */
    eTPU->CHAN[spark_channel_1].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_1 - fs_etpu_data_ram_start) >> 3);

    /* write FM (function mode) bits */
    eTPU->CHAN[spark_channel_1].SCR.R = polarity;

    if (spark_channel_1 < 64 )
    {
        etpu_tcr1_freq_local = etpu_a_tcr1_freq;
    }
    else
    {
        etpu_tcr1_freq_local = etpu_b_tcr1_freq;
    }

    TCR2_Counts_Per_Engine_Cycle = fs_etpu_get_chan_local_24(cam_chan, FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    TCR2_Counts_Per_Engine_Cycle_addr = (uint32_t)((eTPU->CHAN[cam_chan].CR.B.CPBA << 3) + FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    /* initialize SPARK global parameters */
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MIN_DWELL_OFFSET                    - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * min_dwell_time) / 100;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MAX_DWELL_OFFSET                    - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * max_dwell_time) / 100;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MULTI_ON_TIME_OFFSET                - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * multi_on_time) / 100;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MULTI_OFF_TIME_OFFSET               - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * multi_off_time) / 100;
    *( (uint8_t*)fs_etpu_data_ram_start + (FS_ETPU_SPARK_MULTI_NUM_PULSES_OFFSET                   >> 2))  = multi_num_pulses;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_RECALC_OFFSET_ANGLE_OFFSET          - 1) >> 2)) = ((long long)recalc_offset_angle * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1) >> 2)) = (TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF);

    /* initialize SPARK channel parameters */
    *(pba_1 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_1 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1) / 100;                                
    *(pba_1 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_1 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2) / 100;
    *(pba_1 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;

    /* issue HSR to start channel running */
    eTPU->CHAN[spark_channel_1].HSRR.R = FS_ETPU_SPARK_INIT;

    return (error_code);
}

#if __CWCC__
#pragma pop
#endif