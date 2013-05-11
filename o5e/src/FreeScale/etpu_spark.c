/***************************************************************************
* FILE NAME: etpu_spark.c                     COPYRIGHT (c) FREESCALE 2009 *
*                                                      All Rights Reserved *
*==========================================================================*
* ORIGINAL AUTHOR: Stan Ostrum                LAST UPDATE: 09/01/08 14:27  *
*                                                                          *
* DESCRIPTION: This file contains the source code for the eTPU set2 SPARK  *
*              function API.                                               *
*                                                                          *
****************************************************************************
* REVISION HISTORY:
*
* $Log: etpu_spark.c,v $
* Revision 1.6  2009/01/27 15:29:01  r47354
* Added default init parameters for end angles and dwell times.
* Make TCR count calculations less likely to overflow
* Change comments style
*
* Revision 1.5  2009/01/20 13:22:17  r54529
* Return data types changed to int32_t.
*
* Revision 1.4  2009/01/08 22:38:56  ra5622
* Fixed call in fs_etpu_spark_set_recalc_offset_angle routine to update global parameter instead of local parameter.
*
*
* Revision 1.3  2009/01/06 15:17:08  ra5622
* Initial release.
*
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

#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_unusedarg    off
#pragma warn_implicitconv off
#endif

/*******************************************************************************
* FUNCTION: fs_etpu_spark_init_3cylinders
*
* PURPOSE:  initialize 3 eTPU channels to generate SPARK outputs
*
* INPUT PARAMETERS:
*
*   all angle parameters are in units of 1/100 degree with a range of 0 - 71999
*
*   all time parameters are in units of us
*
*   spark_channel_1    - eTPU channel assigned to SPARK for cylinder 1
*   spark_channel_2    - eTPU channel assigned to SPARK for cylinder 2
*   spark_channel_3    - eTPU channel assigned to SPARK for cylinder 3
*   cyl_offset_angle_1 - offset angle for cylinder 1 <0..71999>
*   cyl_offset_angle_2 - offset angle for cylinder 2 <0..71999> 
*   cyl_offset_angle_3 - offset angle for cylinder 3 <0..71999> 
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
int32_t fs_etpu_spark_init_3cylinders(  uint8_t  spark_channel_1,
                                        uint8_t  spark_channel_2,
                                        uint8_t  spark_channel_3,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint24_t cyl_offset_angle_2,
                                        uint24_t cyl_offset_angle_3,
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
    int32_t  error_code = 0;                        /* return value */
    uint32_t *pba_1, *pba_2, *pba_3;                /* channel parameter base addresses */
    uint32_t etpu_tcr1_freq;                        /* TCR1 timebase frequency */
    uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
    uint24_t TCR2_Counts_Per_Engine_Cycle;

    /* disable channels */
    fs_etpu_disable(spark_channel_1);
    fs_etpu_disable(spark_channel_2);
    fs_etpu_disable(spark_channel_3);

    if ((cyl_offset_angle_1 > 72000) || (cyl_offset_angle_2 > 72000) ||
        (cyl_offset_angle_3 > 72000) || (recalc_offset_angle > 72000))
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

    if (eTPU->CHAN[spark_channel_2].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_2 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_2 = fs_etpu_data_ram(spark_channel_2);
    }

    if (eTPU->CHAN[spark_channel_3].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_3 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_3 = fs_etpu_data_ram(spark_channel_3);
    }

    /* write channel configuration registers */
    eTPU->CHAN[spark_channel_1].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_1 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_2].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_2 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_3].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_3 - fs_etpu_data_ram_start) >> 3);

    /* write FM (function mode) bits */
    eTPU->CHAN[spark_channel_1].SCR.R = polarity;
    eTPU->CHAN[spark_channel_2].SCR.R = polarity;
    eTPU->CHAN[spark_channel_3].SCR.R = polarity;

    if (spark_channel_1 < 64 )
    {
        etpu_tcr1_freq = etpu_a_tcr1_freq;
    }
    else
    {
        etpu_tcr1_freq = etpu_b_tcr1_freq;
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
    *(pba_2 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1) / 100;                                
    *(pba_2 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2) / 100;
    *(pba_2 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1) / 100;                                
    *(pba_3 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2) / 100;
    *(pba_3 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_3 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;

    /* issue HSR to start channel running */
    eTPU->CHAN[spark_channel_1].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_2].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_3].HSRR.R = FS_ETPU_SPARK_INIT;

    return (error_code);
}

/*******************************************************************************
* FUNCTION: fs_etpu_spark_init_4cylinders
*
* PURPOSE:  initialize 4 eTPU channels to generate SPARK outputs
*
* INPUT PARAMETERS:
*
*   all angle parameters are in units of 1/100 degree with a range of 0 - 71999
*
*   all time parameters are in units of us
*
*   spark_channel_1       - eTPU channel assigned to SPARK for cylinder 1
*   spark_channel_2       - eTPU channel assigned to SPARK for cylinder 2
*   spark_channel_3       - eTPU channel assigned to SPARK for cylinder 3
*   spark_channel_4       - eTPU channel assigned to SPARK for cylinder 4
*   cyl_offset_angle_1 - offset angle for cylinder 1 <0..71999> 
*   cyl_offset_angle_2 - offset angle for cylinder 2 <0..71999> 
*   cyl_offset_angle_3 - offset angle for cylinder 3 <0..71999> 
*   cyl_offset_angle_4 - offset angle for cylinder 4 <0..71999> 
*   priority           - selects priority for SPARK
*   polarity           - selects output pin polarity for SPARK
*   min_dwell_time     - minimum dwell time in us
*   max_dwell_time     - maximum dwell time in us
*   multi_on_time      - multi spark pulse active time in us
*   multi_off_time     - multi spark pulse inactive time in us
*   multi_num_pulses   - number of multi spark pulses to generate
*   recalc_offset_angle - angle before estimated start angle at which start angle
*                         is recalculated <0..71999> 
*   init_dwell_time_1  - dwell time  in us for first pulse (used for all channels at init)
*   init_dwell_time_2  - dwell time  in us for 2nd pulse (used for all channels at init)
*   init_end_angle_1   - end angle for first pulse (used for all channels at init) <0..71999> 
*   init_end_angle_2   - end angle for 2nd pulse (used for all channels at init) <0..71999>   
*
* RETURN VALUE:  - 0 if SPARK initialization was successful
*                - FS_ETPU_ERROR_VALUE if any angle parameters are out of range
*                - FS_ETPU_ERROR_MALLOC if a parameter RAM allocation error occurs
*
*******************************************************************************/
int32_t fs_etpu_spark_init_4cylinders(  uint8_t  spark_channel_1,
                                        uint8_t  spark_channel_2,
                                        uint8_t  spark_channel_3,
                                        uint8_t  spark_channel_4,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint24_t cyl_offset_angle_2,
                                        uint24_t cyl_offset_angle_3,
                                        uint24_t cyl_offset_angle_4,
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
    int32_t  error_code = 0;                        /* return value */
    uint32_t *pba_1, *pba_2, *pba_3, *pba_4;        /* channel parameter base addresses */
    uint32_t etpu_tcr1_freq;                        /* TCR1 timebase frequency */
    uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
    uint24_t TCR2_Counts_Per_Engine_Cycle;

    /* disable channels */
    fs_etpu_disable(spark_channel_1);
    fs_etpu_disable(spark_channel_2);
    fs_etpu_disable(spark_channel_3);
    fs_etpu_disable(spark_channel_4);

    if ((cyl_offset_angle_1 > 72000) || (cyl_offset_angle_2 > 72000) ||
        (cyl_offset_angle_3 > 72000) || (cyl_offset_angle_4 > 72000) ||
        (recalc_offset_angle > 72000))
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

    if (eTPU->CHAN[spark_channel_2].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_2 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_2 = fs_etpu_data_ram(spark_channel_2);
    }

    if (eTPU->CHAN[spark_channel_3].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_3 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_3 = fs_etpu_data_ram(spark_channel_3);
    }

    if (eTPU->CHAN[spark_channel_4].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_4 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_4 = fs_etpu_data_ram(spark_channel_4);
    }

    /* write channel configuration registers */
    eTPU->CHAN[spark_channel_1].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_1 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_2].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_2 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_3].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_3 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_4].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_4 - fs_etpu_data_ram_start) >> 3);

    /* write FM (function mode) bits */
    eTPU->CHAN[spark_channel_1].SCR.R = polarity;
    eTPU->CHAN[spark_channel_2].SCR.R = polarity;
    eTPU->CHAN[spark_channel_3].SCR.R = polarity;
    eTPU->CHAN[spark_channel_4].SCR.R = polarity;

    if (spark_channel_1 < 64 )
    {
        etpu_tcr1_freq = etpu_a_tcr1_freq;
    }
    else
    {
        etpu_tcr1_freq = etpu_b_tcr1_freq;
    }

    TCR2_Counts_Per_Engine_Cycle = fs_etpu_get_chan_local_24(cam_chan, FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    TCR2_Counts_Per_Engine_Cycle_addr = (uint32_t)((eTPU->CHAN[cam_chan].CR.B.CPBA << 3) + FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    /* initialize SPARK global parameters */
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MIN_DWELL_OFFSET                    - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * min_dwell_time/100);
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MAX_DWELL_OFFSET                    - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * max_dwell_time/100);
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MULTI_ON_TIME_OFFSET                - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * multi_on_time /100);
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MULTI_OFF_TIME_OFFSET               - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * multi_off_time/100);
    *( (uint8_t*)fs_etpu_data_ram_start + (FS_ETPU_SPARK_MULTI_NUM_PULSES_OFFSET                   >> 2))  = multi_num_pulses;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_RECALC_OFFSET_ANGLE_OFFSET          - 1) >> 2)) = ((long long)recalc_offset_angle * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1) >> 2)) = (TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF);

    /* initialize SPARK channel parameters */
    *(pba_1 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_1 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_1 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_1 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100); 
    *(pba_1 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_2 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_2 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_3 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_3 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_3 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_4 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_4 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_4 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_4 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_4 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_4 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;

    /* issue HSR to start channel running */
    eTPU->CHAN[spark_channel_1].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_2].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_3].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_4].HSRR.R = FS_ETPU_SPARK_INIT;

    return (error_code);
}

/*******************************************************************************
* FUNCTION: fs_etpu_spark_init_6cylinders
*
* PURPOSE:  initialize 6 eTPU channels to generate SPARK outputs
*
* INPUT PARAMETERS:
*
*   all angle parameters are in units of 1/100 degree with a range of 0 - 71999
*
*   all time parameters are in units of us
*
*   spark_channel_1       - eTPU channel assigned to SPARK for cylinder 1
*   spark_channel_2       - eTPU channel assigned to SPARK for cylinder 2
*   spark_channel_3       - eTPU channel assigned to SPARK for cylinder 3
*   spark_channel_4       - eTPU channel assigned to SPARK for cylinder 4
*   spark_channel_5       - eTPU channel assigned to SPARK for cylinder 5
*   spark_channel_6       - eTPU channel assigned to SPARK for cylinder 6
*   cyl_offset_angle_1 - offset angle for cylinder 1 <0..71999> 
*   cyl_offset_angle_2 - offset angle for cylinder 2 <0..71999> 
*   cyl_offset_angle_3 - offset angle for cylinder 3 <0..71999> 
*   cyl_offset_angle_4 - offset angle for cylinder 4 <0..71999> 
*   cyl_offset_angle_5 - offset angle for cylinder 5 <0..71999> 
*   cyl_offset_angle_6 - offset angle for cylinder 6 <0..71999> 
*   priority           - selects priority for SPARK
*   polarity           - selects output pin polarity for SPARK
*   min_dwell_time     - minimum dwell time in us
*   max_dwell_time     - maximum dwell time in us
*   multi_on_time      - multi spark pulse active time in us
*   multi_off_time     - multi spark pulse inactive time in us
*   multi_num_pulses   - number of multi spark pulses to generate
*   recalc_offset_angle - angle before estimated start angle at which start angle
*                         is recalculated <0..71999> 
*   init_dwell_time_1  - default dwell time in us for first pulse (used for all channels at init)
*   init_dwell_time_2  - default dwell time in us for 2nd pulse (used for all channels at init)
*   init_end_angle_1   - default end angle for first pulse (used for all channels at init) <0..71999> 
*   init_end_angle_2   - default end angle for 2nd pulse (used for all channels at init) <0..71999>   
*
* RETURN VALUE:  - 0 if SPARK initialization was successful
*                - FS_ETPU_ERROR_VALUE if any angle parameters are out of range
*                - FS_ETPU_ERROR_MALLOC if a parameter RAM allocation error occurs
*
*******************************************************************************/
int32_t fs_etpu_spark_init_6cylinders(  uint8_t  spark_channel_1,
                                        uint8_t  spark_channel_2,
                                        uint8_t  spark_channel_3,
                                        uint8_t  spark_channel_4,
                                        uint8_t  spark_channel_5,
                                        uint8_t  spark_channel_6,
                                        uint8_t  cam_chan,
                                        uint24_t cyl_offset_angle_1,
                                        uint24_t cyl_offset_angle_2,
                                        uint24_t cyl_offset_angle_3,
                                        uint24_t cyl_offset_angle_4,
                                        uint24_t cyl_offset_angle_5,
                                        uint24_t cyl_offset_angle_6,
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
    int32_t  error_code = 0;                                    /* return value */
    uint32_t *pba_1, *pba_2, *pba_3, *pba_4, *pba_5, *pba_6;    /* channel parameter base addresses */
    uint32_t etpu_tcr1_freq;                                    /* TCR1 timebase frequency */
    uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
    uint24_t TCR2_Counts_Per_Engine_Cycle;

    /* disable channels */
    fs_etpu_disable(spark_channel_1);
    fs_etpu_disable(spark_channel_2);
    fs_etpu_disable(spark_channel_3);
    fs_etpu_disable(spark_channel_4);
    fs_etpu_disable(spark_channel_5);
    fs_etpu_disable(spark_channel_6);

    if ((cyl_offset_angle_1 > 72000) || (cyl_offset_angle_2 > 72000) ||
        (cyl_offset_angle_3 > 72000) || (cyl_offset_angle_4 > 72000) ||
        (cyl_offset_angle_5 > 72000) || (cyl_offset_angle_6 > 72000) ||
        (recalc_offset_angle > 72000))
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

    if (eTPU->CHAN[spark_channel_2].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_2 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_2 = fs_etpu_data_ram(spark_channel_2);
    }

    if (eTPU->CHAN[spark_channel_3].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_3 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_3 = fs_etpu_data_ram(spark_channel_3);
    }

    if (eTPU->CHAN[spark_channel_4].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_4 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_4 = fs_etpu_data_ram(spark_channel_4);
    }

    if (eTPU->CHAN[spark_channel_5].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_5 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_5 = fs_etpu_data_ram(spark_channel_5);
    }

    if (eTPU->CHAN[spark_channel_6].CR.B.CPBA == 0 )
    {
        /* get number of parameters used by eTPU C code */
        if ((pba_6 = fs_etpu_malloc(FS_ETPU_SPARK_NUM_PARMS)) == 0)
        {
            return (FS_ETPU_ERROR_MALLOC);
        }
    }
    else    /* set pba to what is in the CR register */
    {
        pba_6 = fs_etpu_data_ram(spark_channel_6);
    }

    /* write channel configuration registers */
    eTPU->CHAN[spark_channel_1].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_1 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_2].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_2 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_3].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_3 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_4].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_4 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_5].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_5 - fs_etpu_data_ram_start) >> 3);
    eTPU->CHAN[spark_channel_6].CR.R = (priority << 28) +
                                    (FS_ETPU_SPARK_TABLE_SELECT << 24) +
                                    (FS_ETPU_SPARK_FUNCTION_NUMBER << 16) +
                                    (((uint32_t)pba_6 - fs_etpu_data_ram_start) >> 3);

    /* write FM (function mode) bits */
    eTPU->CHAN[spark_channel_1].SCR.R = polarity;
    eTPU->CHAN[spark_channel_2].SCR.R = polarity;
    eTPU->CHAN[spark_channel_3].SCR.R = polarity;
    eTPU->CHAN[spark_channel_4].SCR.R = polarity;
    eTPU->CHAN[spark_channel_5].SCR.R = polarity;
    eTPU->CHAN[spark_channel_6].SCR.R = polarity;

    if (spark_channel_1 < 64 )
    {
        etpu_tcr1_freq = etpu_a_tcr1_freq;
    }
    else
    {
        etpu_tcr1_freq = etpu_b_tcr1_freq;
    }

    TCR2_Counts_Per_Engine_Cycle = fs_etpu_get_chan_local_24(cam_chan, FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    TCR2_Counts_Per_Engine_Cycle_addr = (uint32_t)((eTPU->CHAN[cam_chan].CR.B.CPBA << 3) + FS_ETPU_CAM_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    /* initialize SPARK global parameters */
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MIN_DWELL_OFFSET                    - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * min_dwell_time/100);
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MAX_DWELL_OFFSET                    - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * max_dwell_time/100);
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MULTI_ON_TIME_OFFSET                - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * multi_on_time /100);
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_MULTI_OFF_TIME_OFFSET               - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * multi_off_time/100);
    *( (uint8_t*)fs_etpu_data_ram_start + (FS_ETPU_SPARK_MULTI_NUM_PULSES_OFFSET                   >> 2))  = multi_num_pulses;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_RECALC_OFFSET_ANGLE_OFFSET          - 1) >> 2)) = ((long long)recalc_offset_angle * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *((uint32_t*)fs_etpu_data_ram_start + ((FS_ETPU_SPARK_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET - 1) >> 2)) = (TCR2_Counts_Per_Engine_Cycle_addr & 0x1FFF);

    /* initialize SPARK channel parameters */
    *(pba_1 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_1 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_1 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_1 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_1 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_2 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_2 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_2 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_3 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_3 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_3 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_3 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_4 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_4 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_4 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_4 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_4 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_4 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_5 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_5 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_5 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_5 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_5 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_5 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_6 + ((FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_6 + ((FS_ETPU_SPARK_SPARK1_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_1 / 100);                                
    *(pba_6 + ((FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET           - 1) >> 2)) = ((long long)init_end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;
    *(pba_6 + ((FS_ETPU_SPARK_SPARK2_DWELL_OFFSET           - 1) >> 2)) = ((etpu_tcr1_freq / 10000) * init_dwell_time_2 / 100);
    *(pba_6 + ((FS_ETPU_SPARK_CYLINDER_OFFSET_ANGLE_OFFSET  - 1) >> 2)) = ((long long)cyl_offset_angle_6 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000;

    /* issue HSR to start channel running */
    eTPU->CHAN[spark_channel_1].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_2].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_3].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_4].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_5].HSRR.R = FS_ETPU_SPARK_INIT;
    eTPU->CHAN[spark_channel_6].HSRR.R = FS_ETPU_SPARK_INIT;

    return (error_code);
}

/*******************************************************************************
* FUNCTION: fs_etpu_spark_set_dwell_times
*
* PURPOSE: update the spark normal pulse dwell times
*
* INPUT PARAMETERS:
*
*   channel             - eTPU SPARK channel number
*   spark1_dwell_time   - targeted dwell time for spark1 normal pulse in us
*   spark2_dwell_time   - targeted dwell time for spark2 normal pulse in us
*
* RETURN VALUES: - 0 if dwell time update was successful
*                - sum of pending HSR numbers if the channel has pending HSRs
*                       (in this case, the function should be called again)
*
*******************************************************************************/
int32_t fs_etpu_spark_set_dwell_times( uint8_t  channel,
                                       uint24_t dwell_time_1,
                                       uint24_t dwell_time_2 )
{
    uint32_t etpu_tcr1_freq;
    uint32_t hsrr;

    if (channel < 64)
        etpu_tcr1_freq = etpu_a_tcr1_freq;
    else
        etpu_tcr1_freq = etpu_b_tcr1_freq;

    fs_etpu_set_chan_local_24(channel, FS_ETPU_SPARK_SPARK1_DWELL_OFFSET, ((etpu_tcr1_freq / 10000) * dwell_time_1 / 100));
    fs_etpu_set_chan_local_24(channel, FS_ETPU_SPARK_SPARK2_DWELL_OFFSET, ((etpu_tcr1_freq / 10000) * dwell_time_2 / 100));

    /* if the channel has pending HSRs, don't write new HSR but instead return sum of pending HSRs */
    hsrr = eTPU->CHAN[channel].HSRR.R;
    if (hsrr != 0)
    {
        return ((int32_t)hsrr);
    }

    /* write spark update HSR */
    eTPU->CHAN[channel].HSRR.R = FS_ETPU_SPARK_UPDATE;


    return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_spark_set_end_angles
*
* PURPOSE: update the spark normal pulse end angles
*
* INPUT PARAMETERS:
*
*   channel             - eTPU SPARK channel number
*   spark1_end_angle    - normal pulse end angle for spark1 in 1/100 degree (range is 0 to 71999)
*   spark2_end_angle    - normal pulse end angle for spark2 in 1/100 degree (range is 0 to 71999)
*
* RETURN VALUES: - 0 if end angle update was successful
*                - FS_ETPU_ERROR_VALUE if either spark end angle parameter is out of range
*                - sum of pending HSR numbers if the channel has pending HSRs
*                       (in this case, the function should be called again)
*
*******************************************************************************/
int32_t fs_etpu_spark_set_end_angles( uint8_t  channel,
                                      uint24_t end_angle_1,
                                      uint24_t end_angle_2)
{
    uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
    uint24_t TCR2_Counts_Per_Engine_Cycle;
    uint32_t hsrr;

    if ((end_angle_1 > 72000) || (end_angle_2 > 72000))
    {
        return (FS_ETPU_ERROR_VALUE);
    }

    TCR2_Counts_Per_Engine_Cycle_addr =
            fs_etpu_get_global_24(FS_ETPU_SPARK_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    TCR2_Counts_Per_Engine_Cycle = *((uint32_t*)(fs_etpu_data_ram_start + TCR2_Counts_Per_Engine_Cycle_addr - 1));
    TCR2_Counts_Per_Engine_Cycle &= 0xFFFFFF;

    fs_etpu_set_chan_local_24(channel, FS_ETPU_SPARK_SPARK1_ANGLE_OFFSET,
                ((long long)end_angle_1 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000);

    fs_etpu_set_chan_local_24(channel, FS_ETPU_SPARK_SPARK2_ANGLE_OFFSET,
                ((long long)end_angle_2 * TCR2_Counts_Per_Engine_Cycle) / (long long)72000);

    /* if the channel has pending HSRs, don't write new HSR but instead return sum of pending HSRs */
    hsrr = eTPU->CHAN[channel].HSRR.R;
    if (hsrr != 0)
    {
        return ((int32_t)hsrr);
    }

    /* write spark update HSR */
    eTPU->CHAN[channel].HSRR.R = FS_ETPU_SPARK_UPDATE;

  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_spark_set_recalc_offset_angle
*
* PURPOSE: update the global start angle recalculation offset angle
*
* INPUT PARAMETERS:
*
*   channel             - eTPU SPARK channel number
*   recalc_offset_angle - angle before estimated start angle at which start angle
                            is recalculated in 1/100 degree (range is 0 to 71999)
*
* RETURN VALUES: - 0 if recalc offset angle update was successful
*                - FS_ETPU_ERROR_VALUE if recalc offset angle parameter is out of range
*
*******************************************************************************/
int32_t fs_etpu_spark_set_recalc_offset_angle( uint8_t  channel, 
                                               uint24_t recalc_offset_angle)
{
    uint32_t TCR2_Counts_Per_Engine_Cycle_addr;
    uint24_t TCR2_Counts_Per_Engine_Cycle;

    if (recalc_offset_angle > 72000)
    {
        return (FS_ETPU_ERROR_VALUE);
    }

    TCR2_Counts_Per_Engine_Cycle_addr =
            fs_etpu_get_global_24(FS_ETPU_SPARK_TCR2_COUNTS_PER_ENGINE_CYCLE_OFFSET);

    TCR2_Counts_Per_Engine_Cycle = *((uint32_t*)(fs_etpu_data_ram_start + TCR2_Counts_Per_Engine_Cycle_addr - 1));
    TCR2_Counts_Per_Engine_Cycle &= 0xFFFFFF;

    fs_etpu_set_global_24(FS_ETPU_SPARK_RECALC_OFFSET_ANGLE_OFFSET,
                ((long long)recalc_offset_angle * TCR2_Counts_Per_Engine_Cycle) / (long long)72000);

  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_spark_set_min_max_dwell_times
*
* PURPOSE: update the minimum and maximum dwell times
*
* INPUT PARAMETERS:
*
*   channel        - eTPU SPARK channel number
*   min_dwell_time - minimum dwell time in us
*   max_dwell_time - maximum dwell time in us
*
* RETURN VALUES: - always returns 0
*
*******************************************************************************/
int32_t fs_etpu_spark_set_min_max_dwell_times( uint8_t  channel,
                                               uint24_t min_dwell_time,
                                               uint24_t max_dwell_time)
{
  uint32_t etpu_tcr1_freq;

  if (channel<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;

  fs_etpu_set_global_24(FS_ETPU_SPARK_MIN_DWELL_OFFSET, ((etpu_tcr1_freq / 10000) * min_dwell_time / 100));
  fs_etpu_set_global_24(FS_ETPU_SPARK_MAX_DWELL_OFFSET, ((etpu_tcr1_freq / 10000) * max_dwell_time / 100));

  return(0);
}

/*******************************************************************************
* FUNCTION: fs_etpu_spark_set_multi_pulses
*
* PURPOSE: update the multi spark pulse times and number of pulses
*
* INPUT PARAMETERS:
*
*   channel             - eTPU SPARK channel number
*   multi_on_time       - multi spark pulse active time in us
*   multi_off_time      - multi spark pulse inactive time in us
*   multi_num_pulses    - number of multi spark pulses to generate
*
* RETURN VALUES: - always returns 0
*
*******************************************************************************/
int32_t fs_etpu_spark_set_multi_pulses( uint8_t  channel, 
                                        uint24_t multi_on_time,
                                        uint24_t multi_off_time, 
                                        uint8_t  multi_num_pulses)
{
  uint32_t etpu_tcr1_freq;

  if (channel<64)
     etpu_tcr1_freq = etpu_a_tcr1_freq;
  else
     etpu_tcr1_freq = etpu_b_tcr1_freq;

  fs_etpu_set_global_24(FS_ETPU_SPARK_MULTI_ON_TIME_OFFSET, ((etpu_tcr1_freq / 10000) * multi_on_time / 100));
  fs_etpu_set_global_24(FS_ETPU_SPARK_MULTI_OFF_TIME_OFFSET, ((etpu_tcr1_freq / 10000) * multi_off_time / 100));
  fs_etpu_set_global_8(FS_ETPU_SPARK_MULTI_NUM_PULSES_OFFSET, multi_num_pulses);

  return(0);
}

/*********************************************************************
 *
 * Copyright:
 *	Freescale Semiconductor, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is retained
 *  without alteration in any modified and/or redistributed versions,
 *  and that such modified versions are clearly identified as such. No
 *  licenses are granted by implication, estoppel or otherwise under
 *  any patents or trademarks of Freescale Semiconductor, Inc. This
 *  software is provided on an "AS IS" basis and without warranty.
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