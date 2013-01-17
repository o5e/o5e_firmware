/*******************************************************************************
* FILE NAME: etpu_toothgen.c        COPYRIGHT (c) Freescale Semiconductor 2006
*                                               All Rights Reserved
* DESCRIPTION:
* This file contains the prototypes and definitions for the eTPU Function
* Tooth Generator (ToothGen) API.
*===============================================================================
* REV      AUTHOR      DATE        DESCRIPTION OF CHANGE
* ---   -----------  ----------    ---------------------
* 0.0   M  Li        19/Jan/04     Initial creation
* 0.1   S. Mihalik   07/Jun/06     Modified for eTPU Utils conformance
*                                  based on eTPU Set 1 PWM API design
* 1.0   M. Brejl     07/Sep/06     Complete rework. New features.
* 1.1   M. Brejl     28/Mar/08     DoxyGen keywords removed.
* 1.2   M. Brejl     03/Apr/08     Some parameters changed.
* 1.3   M. Brejl     13/May/08     2nd missing tooth and tooth in gap added.
* 1.4   M. Brejl     28/Aug/08     tooth_number param check error corrected.
* 1.5   M. Brejl     27/Nov/08     use of ufract24_t, correction of comments.
*******************************************************************************/

#include "etpu_util.h"          /* Utility routines for working with the eTPU */
#include "etpu_toothgen.h"      /* eTPU Tooth Gen API defines */

/*******************************************************************************
* Global variables
*******************************************************************************/
extern uint32_t fs_etpu_data_ram_start;
extern uint32_t fs_etpu_data_ram_ext;

/*******************************************************************************
* Constants and macros
*******************************************************************************/

/*******************************************************************************
* Local types
*******************************************************************************/

/*******************************************************************************
* Local function prototypes
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
* Global functions
*******************************************************************************/
#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif
/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_init
*
* PURPOSE: Initialize two eTPU channels to generate engine crank and cam
*          tooth pattern.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_crank      - The number of the channel which generates the crank 
*                        signal
*   channel_cam        - The number of the channel which generates the cam
*                        signal
*   priority           - The priority to assign to both channels
*   crank_number_of_physical_teeth - number of physical teeth on the crank wheel 
*                        <1 to 252>.
*   crank_number_of_missing_teeth - number of missing teeth on the crank wheel
*                        <1, 2 or 3>.
*   tooth_duty_cycle   - Fraction of the tooth period when the tooth is high
*                        <0x000000..0xFFFFFF> corresponds to <0%..100%>
*   tooth_number       - Tooth number of the first crank tooth to generate
*        <1..2*(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   engine_speed_rpm   - Engine speed as a number of crank revolutions per
*                        minute <10..100000>
*   tcr1_timebase_freq - The frequency of the TCR1 timebase in Hz 
*   cam_start          - The tooth number at which the cam signal goes high
*        <1..2*(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   cam_stop           - The tooth number at which the cam signal goes low
*        <1..2*(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*
* RETURNS NOTES: 
* The function returns a non-zero error code in case the eTPU channels can not 
* be initialized or the eTPU channel parameters need to be limited into 
* an appropriate range. 
* Error codes which can be returned are: FS_ETPU_ERROR_VALUE
*                                        FS_ETPU_ERROR_MALLOC
*
*******************************************************************************/
int32_t fs_etpu_toothgen_init( uint8_t channel_crank,
                               uint8_t channel_cam,
                               uint8_t priority,
                               uint8_t crank_number_of_physical_teeth,
                               uint8_t crank_number_of_missing_teeth,
                            ufract24_t tooth_duty_cycle,
                               uint8_t tooth_number,
                              uint32_t engine_speed_rpm,
                              uint32_t tcr1_timebase_freq,
                               uint8_t cam_start,
                               uint8_t cam_stop )
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba;          /* channel parameter base address */
  uint32_t cr;
  uint8_t teeth_per_rev;
  uint24_t tooth_period;

  /* 1. DISABLE CHANNELS TO ASSIGN FUNCTIONS SAFELY */
  fs_etpu_disable( channel_crank );
  fs_etpu_disable( channel_cam );

  /* 2.  ALLOCATE PARAMETER RAM FOR CHANNEL */
  if (eTPU->CHAN[channel_crank].CR.B.CPBA == 0 )
  {
    /* get parameter RAM - number of parameters passed from eTPU C code */
    pba = fs_etpu_malloc(FS_ETPU_TOOTHGEN_NUM_PARMS);

    if (pba == 0)
    {
      return (FS_ETPU_ERROR_MALLOC);
    }
  }
  else /*set pba to what is in the CR register*/
  {
    pba = fs_etpu_data_ram(channel_crank);
  }

  /* 3. CHECK FOR VALID INPUT PARAMETERS AND WRITE TO PARAMETER RAM */
  if (crank_number_of_missing_teeth > 3)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_number_of_missing_teeth = 3;
  }
  else if (crank_number_of_missing_teeth == 0)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_number_of_missing_teeth = 1;
  }

  if (crank_number_of_physical_teeth > 255 - crank_number_of_missing_teeth)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_number_of_physical_teeth = 255 - crank_number_of_missing_teeth;
  }
  teeth_per_rev = crank_number_of_physical_teeth +crank_number_of_missing_teeth;
  *(pba + ((FS_ETPU_TOOTHGEN_TEETHPERREV_OFFSET - 1)>>2)) = teeth_per_rev;

  *((uint8_t*)pba + FS_ETPU_TOOTHGEN_TEETHMISSING_OFFSET) = 
    crank_number_of_missing_teeth;

  if (tooth_duty_cycle > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    tooth_duty_cycle = 0xFFFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_TOOTHDUTYCYCLE_OFFSET - 1)>>2)) = tooth_duty_cycle;

  if (tooth_number > 2*teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    tooth_number = 2*teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_TOOTHNUMBER_OFFSET - 1)>>2)) = tooth_number;

  tooth_period = tcr1_timebase_freq/teeth_per_rev*60/engine_speed_rpm;
  if (tooth_period > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    tooth_period = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_TOOTHPERIOD_OFFSET - 1)>>2)) = tooth_period;
  *(pba + ((FS_ETPU_TOOTHGEN_TOOTHPERIODNEW_OFFSET - 1)>>2)) = tooth_period;

  if (cam_start > 2*teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_start = 2*teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMSTART_OFFSET - 1)>>2)) = cam_start;

  if (cam_stop > 2*teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_stop = 2*teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMSTOP_OFFSET - 1)>>2)) = cam_stop;

  *(pba + ((FS_ETPU_TOOTHGEN_ACCELRATE_OFFSET - 1)>>2)) = 0;
  *(pba + ((FS_ETPU_TOOTHGEN_MISSINGTOOTH_OFFSET - 1)>>2)) = 0;
  *(pba + ((FS_ETPU_TOOTHGEN_NOISETOOTH_OFFSET - 1)>>2)) = 0;
  *(pba + ((FS_ETPU_TOOTHGEN_NOISEPULSESHIFT_OFFSET - 1)>>2)) = 0;
  *(pba + ((FS_ETPU_TOOTHGEN_NOISEPULSEWIDTH_OFFSET - 1)>>2)) = 0;
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISETOOTH_OFFSET - 1)>>2)) = 0;
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISEPULSESHIFT_OFFSET - 1)>>2)) = 0;
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISEPULSEWIDTH_OFFSET - 1)>>2)) = 0;
  *((uint8_t*)pba + FS_ETPU_TOOTHGEN_CAMCHANNEL_OFFSET) = channel_cam;
  *((uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET) = 0;

  /* 4. WRITE TO CHANNEL CONFIGURATION REGISTER */
  cr = (priority << 28) + (FS_ETPU_TOOTHGEN_TABLE_SELECT << 24) +
       (FS_ETPU_TOOTHGEN_FUNCTION_NUMBER << 16) +
       (((uint32_t)pba - fs_etpu_data_ram_start)>>3);
  eTPU->CHAN[channel_crank].CR.R = cr;
  eTPU->CHAN[channel_cam].CR.R = cr;

  /* 5. WRITE FUNCTION MODE (FM) BITS */
  eTPU->CHAN[channel_crank].SCR.R = FS_ETPU_TOOTHGEN_FM0_CRANK_CHAN;
  eTPU->CHAN[channel_cam].SCR.R = FS_ETPU_TOOTHGEN_FM0_CAM_CHAN;

  /* 6. WRITE HOST SERVICE REQUEST TO START CHANNEL RUNNING */
  eTPU->CHAN[channel_crank].HSRR.R = FS_ETPU_TOOTHGEN_HSR_INIT;
  eTPU->CHAN[channel_cam].HSRR.R = FS_ETPU_TOOTHGEN_HSR_INIT;

  return(error_code);
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_adj
*
* PURPOSE: Addjust parameter to generate an engine acceleration or deceleration
*          profile.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_crank      - The number of the channel which generates
*                        the crank signal
*   accel_rate         - The FRACTION of the difference between the
*                        actual tooth period and the target tooth period
*                        that is added to the tooth period at each tooth.
*                        <0x000000..0xFFFFFF> corresponds to <0%..100%>.
*                        This implements a rough exponential change of speed
*   target_engine_speed_rpm - Target engine speed as a number of crank 
*                        revolutions per minute <10..100000>
*   tcr1_timebase_freq - The frequency of the TCR1 timebase in Hz 
*
* RETURNS NOTES: 
* The function returns a non-zero error code in case the eTPU channel parameters 
* need to be limited into an appropriate range. 
* Error code which can be returned is: FS_ETPU_ERROR_VALUE
*
*******************************************************************************/
int32_t fs_etpu_toothgen_adj( uint8_t channel_crank,
                           ufract24_t accel_rate,
                             uint32_t target_engine_speed_rpm,
                             uint32_t tcr1_timebase_freq )
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba;          /* channel parameter base address */
  uint24_t tooth_period_new;
  uint8_t teeth_per_rev;

  pba = fs_etpu_data_ram(channel_crank);

  if (accel_rate > 0xFFFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    accel_rate = 0xFFFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_ACCELRATE_OFFSET - 1)>>2)) = accel_rate;

  teeth_per_rev = *(pba + ((FS_ETPU_TOOTHGEN_TEETHPERREV_OFFSET - 1)>>2));
  tooth_period_new =tcr1_timebase_freq/teeth_per_rev*60/target_engine_speed_rpm;
  if (tooth_period_new > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    tooth_period_new = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_TOOTHPERIODNEW_OFFSET - 1)>>2)) = tooth_period_new;

  return(error_code);
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_set_missing_tooth
*
* PURPOSE: Activats one kind of error mimics - missing crank tooth.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_crank          - The number of the channel which generates
*                            the crank signal
*   error_type             - The type of the missing tooth error - assign one of
*                            FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME
*                            FS_ETPU_TOOTHGEN_ERROR_MISSING_HIGHTIME
*   missing_tooth_number_1 - Number of the first missing tooth 
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   missing_tooth_number_2 - Number of the second missing tooth 
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*                            or 0 if not applicable
*
* The function returns a non-zero error code in case the eTPU channel parameters 
* need to be limited into an appropriate range. 
* Error code which can be returned is: FS_ETPU_ERROR_VALUE
*
*******************************************************************************/
int32_t fs_etpu_toothgen_set_missing_tooth( uint8_t channel_crank,
                                            uint8_t error_type,
                                            uint8_t missing_tooth_number_1,
                                            uint8_t missing_tooth_number_2 )
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;
  uint8_t teeth_per_rev;

  pba = fs_etpu_data_ram(channel_crank);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type &= ~(FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME
                      | FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME);

  teeth_per_rev = 
    (uint8_t)(*(pba + ((FS_ETPU_TOOTHGEN_TEETHPERREV_OFFSET - 1)>>2))); 
  if (missing_tooth_number_1 > teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    missing_tooth_number_1 = teeth_per_rev;
  }
  if (missing_tooth_number_2 > teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    missing_tooth_number_2 = teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_MISSINGTOOTH_OFFSET-1)>>2)) = 
      missing_tooth_number_1;
  *(pba + ((FS_ETPU_TOOTHGEN_MISSINGTOOTH2_OFFSET-1)>>2)) = 
      missing_tooth_number_2;

  *pa_error_type |= error_type & (FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME
                                  | FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME);

  return(error_code);
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_clear_missing_tooth
*
* PURPOSE: Deactivats one kind of error mimics - missing crank tooth.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_crank   - The number of the channel which generates
*                     the crank signal
*
*******************************************************************************/
void fs_etpu_toothgen_clear_missing_tooth( uint8_t channel_crank )
{
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;

  pba = fs_etpu_data_ram(channel_crank);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type &= ~(FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME
                      | FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME);
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_set_crank_noise
*
* PURPOSE: Activats one kind of error mimics - crank noise.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_crank      - The number of the channel which generates
*                        the crank signal
*   noise_tooth_number - Number of tooth where the crank noise appears
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   noise_pulse_shift  - Crank noise pulse shift from tooth rising edge
*                        as TCR1 count.
*   noise_pulse_width  - Crank noise pulse width as TCR1 count.
*
* The function returns a non-zero error code in case the eTPU channel parameters 
* need to be limited into an appropriate range. 
* Error code which can be returned is: FS_ETPU_ERROR_VALUE
*
*******************************************************************************/
int32_t fs_etpu_toothgen_set_crank_noise( uint8_t channel_crank,
                                          uint8_t noise_tooth_number,
                                         uint24_t noise_pulse_shift,
                                         uint24_t noise_pulse_width )
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;
  uint8_t teeth_per_rev;

  pba = fs_etpu_data_ram(channel_crank);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;

  teeth_per_rev = 
    (uint8_t)(*(pba + ((FS_ETPU_TOOTHGEN_TEETHPERREV_OFFSET - 1)>>2))); 
  if (noise_tooth_number > teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    noise_tooth_number = teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_NOISETOOTH_OFFSET - 1)>>2)) = noise_tooth_number;

  if (noise_pulse_shift > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    noise_pulse_shift = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_NOISEPULSESHIFT_OFFSET - 1)>>2)) = noise_pulse_shift;

  if (noise_pulse_width > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    noise_pulse_width = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_NOISEPULSEWIDTH_OFFSET - 1)>>2)) = noise_pulse_width;

  *pa_error_type |= FS_ETPU_TOOTHGEN_ERROR_CRANKNOISE;

  return(error_code);
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_clear_crank_noise
*
* PURPOSE: Deactivats one kind of error mimics - crank noise.
*                
* INPUTS NOTES: This function has the following parameters:
*   channel_crank   - The number of the channel which generates
*                     the crank signal
*
*******************************************************************************/
void fs_etpu_toothgen_clear_crank_noise( uint8_t channel_crank )
{
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;

  pba = fs_etpu_data_ram(channel_crank);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type &= ~FS_ETPU_TOOTHGEN_ERROR_CRANKNOISE;
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_set_missing_cam_pulse
*
* PURPOSE: Activats one kind of error mimics - missing cam pulse.
*
* INPUTS NOTES: This function has the following parameters:
*   channel_crank   - The number of the channel which generates
*                     the crank signal
*
*******************************************************************************/
void fs_etpu_toothgen_set_missing_cam_pulse( uint8_t channel_cam )
{
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;

  pba = fs_etpu_data_ram(channel_cam);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type |= FS_ETPU_TOOTHGEN_ERROR_MISSING_CAM;
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_clear_missing_cam_pulse
*
* PURPOSE: Deactivats one kind of error mimics - missing cam pulse.
*
* INPUTS NOTES: This function has the following parameters:
*   channel_crank   - The number of the channel which generates
*                     the crank signal
*
*******************************************************************************/
void fs_etpu_toothgen_clear_missing_cam_pulse( uint8_t channel_crank )
{
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;

  pba = fs_etpu_data_ram(channel_crank);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type &= ~FS_ETPU_TOOTHGEN_ERROR_MISSING_CAM;
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_set_cam_noise
*
* PURPOSE: Activats one kind of error mimics - cam noise.
*
* INPUTS NOTES: This function has the following parameters:
*   channel_cam        - The number of the channel which generates
*                        the cam signal
*   noise_tooth_number - Number of tooth where the cam noise appears
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   noise_pulse_shift  - Cam noise pulse shift from tooth rising edge
*                        as TCR1 count.
*   noise_pulse_width  - Cam noise pulse width as TCR1 count.
*
* The function returns a non-zero error code in case the eTPU channel parameters 
* need to be limited into an appropriate range. 
* Error code which can be returned is: FS_ETPU_ERROR_VALUE
*
*******************************************************************************/
int32_t fs_etpu_toothgen_set_cam_noise( uint8_t channel_cam,
                                        uint8_t noise_tooth_number,
                                       uint24_t noise_pulse_shift,
                                       uint24_t noise_pulse_width )
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;
  uint8_t teeth_per_rev;

  pba = fs_etpu_data_ram(channel_cam);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;

  teeth_per_rev = 
    (uint8_t)(*(pba + ((FS_ETPU_TOOTHGEN_TEETHPERREV_OFFSET - 1)>>2))); 
  if (noise_tooth_number > 2*teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    noise_tooth_number = 2*teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISETOOTH_OFFSET - 1)>>2)) = noise_tooth_number;

  if (noise_pulse_shift > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    noise_pulse_shift = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISEPULSESHIFT_OFFSET - 1)>>2)) = noise_pulse_shift;

  if (noise_pulse_width > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    noise_pulse_width = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISEPULSEWIDTH_OFFSET - 1)>>2)) = noise_pulse_width;

  *pa_error_type |= FS_ETPU_TOOTHGEN_ERROR_CAMNOISE;

  return(error_code);
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_clear_cam_noise
*
* PURPOSE: Deactivats one kind of error mimics - cam noise.
*
* INPUTS NOTES: This function has the following parameters:
*   channel_crank   - The number of the channel which generates
*                     the crank signal
*
*******************************************************************************/
void fs_etpu_toothgen_clear_cam_noise( uint8_t channel_cam )
{
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;

  pba = fs_etpu_data_ram(channel_cam);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type &= ~FS_ETPU_TOOTHGEN_ERROR_CAMNOISE;
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_set_tooth_in_gap
*
* PURPOSE: Activats one kind of error mimics - tooth in gap.
*
* INPUTS NOTES: This function has the following parameters:
*   channel_crank   - The number of the channel which generates
*                     the crank signal
*
*******************************************************************************/
void fs_etpu_toothgen_set_tooth_in_gap( uint8_t channel_cam )
{
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;

  pba = fs_etpu_data_ram(channel_cam);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type |= FS_ETPU_TOOTHGEN_ERROR_TOOTH_IN_GAP;
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_clear_tooth_in_gap
*
* PURPOSE: Deactivats one kind of error mimics - tooth in gap.
*
* INPUTS NOTES: This function has the following parameters:
*   channel_crank   - The number of the channel which generates
*                     the crank signal
*
*******************************************************************************/
void fs_etpu_toothgen_clear_tooth_in_gap( uint8_t channel_crank )
{
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;

  pba = fs_etpu_data_ram(channel_crank);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type &= ~FS_ETPU_TOOTHGEN_ERROR_TOOTH_IN_GAP;
}


/*******************************************************************************
* FUNCTION: fs_etpu_toothgen_set_error_mimics
*
* PURPOSE: Update all kinds of error mimics at once.
*
* INPUTS NOTES: This function has the following parameters:
*   channel_crank            - The number of the channel which generates  
*                              the crank signal                           
*   error_list               - The list of active errors - select from    
*                              FS_ETPU_TOOTHGEN_ERROR_MISSING_LOWTIME     
*                              FS_ETPU_TOOTHGEN_ERROR_MISSING_HIGHTIME    
*                              FS_ETPU_TOOTHGEN_ERROR_CRANKNOISE          
*                              FS_ETPU_TOOTHGEN_ERROR_MISSING_CAM         
*                              FS_ETPU_TOOTHGEN_ERROR_CAMNOISE
*                              FS_ETPU_TOOTHGEN_ERROR_TOOTH_IN_GAP            
*                              or write 0 to deactivate all errors.       
*   missing_tooth_number_1   - Number of the first missing tooth 
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   missing_tooth_number_2   - Number of the second missing tooth 
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*                              or 0 if not applicable
*   crank_noise_tooth_number - Number of tooth where the crank noise appears
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   crank_noise_pulse_shift  - Crank noise pulse shift from tooth rising
*                              edge as TCR1 count.
*   crank_noise_pulse_width  - Crank noise pulse width as TCR1 count.
*   cam_noise_tooth_number   - Number of tooth where the cam noise appears
*          <1..(crank_number_of_physical_teeth + crank_number_of_missing_teeth)>
*   cam_noise_pulse_shift    - Cam noise pulse shift from tooth rising edge
*                              as TCR1 count.
*   cam_noise_pulse_width    - Cam noise pulse width as TCR1 count.
*
* The function returns a non-zero error code in case the eTPU channel parameters 
* need to be limited into an appropriate range. 
* Error code which can be returned is: FS_ETPU_ERROR_VALUE
*
*******************************************************************************/
int32_t fs_etpu_toothgen_set_error_mimics( uint8_t channel_crank,
                                           uint8_t error_list,
                                           uint8_t missing_tooth_number_1,
                                           uint8_t missing_tooth_number_2,
                                           uint8_t crank_noise_tooth_number,
                                          uint24_t crank_noise_pulse_shift,
                                          uint24_t crank_noise_pulse_width,
                                           uint8_t cam_noise_tooth_number,
                                          uint24_t cam_noise_pulse_shift,
                                          uint24_t cam_noise_pulse_width )
{
  int32_t error_code = 0; /* returned value from etpu API functions */
  uint32_t *pba;          /* channel parameter base address */
  uint8_t *pa_error_type;
  uint8_t teeth_per_rev;

  pba = fs_etpu_data_ram(channel_crank);
  pa_error_type = (uint8_t*)pba + FS_ETPU_TOOTHGEN_ERRORSWITCH_OFFSET;
  *pa_error_type = error_list;

  teeth_per_rev = 
    (uint8_t)(*(pba + ((FS_ETPU_TOOTHGEN_TEETHPERREV_OFFSET - 1)>>2))); 
  if (missing_tooth_number_1 > teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    missing_tooth_number_1 = teeth_per_rev;
  }
  if (missing_tooth_number_2 > teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    missing_tooth_number_2 = teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_MISSINGTOOTH_OFFSET-1)>>2)) = 
      missing_tooth_number_1;
  *(pba + ((FS_ETPU_TOOTHGEN_MISSINGTOOTH2_OFFSET-1)>>2)) = 
      missing_tooth_number_2;

  if (crank_noise_tooth_number > teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_noise_tooth_number = teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_NOISETOOTH_OFFSET - 1)>>2)) = crank_noise_tooth_number;

  if (crank_noise_pulse_shift > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_noise_pulse_shift = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_NOISEPULSESHIFT_OFFSET - 1)>>2)) = crank_noise_pulse_shift;

  if (crank_noise_pulse_width > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    crank_noise_pulse_width = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_NOISEPULSEWIDTH_OFFSET - 1)>>2)) = crank_noise_pulse_width;

  if (cam_noise_tooth_number > 2*teeth_per_rev)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_noise_tooth_number = 2*teeth_per_rev;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISETOOTH_OFFSET - 1)>>2)) = cam_noise_tooth_number;

  if (cam_noise_pulse_shift > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_noise_pulse_shift = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISEPULSESHIFT_OFFSET - 1)>>2)) = cam_noise_pulse_shift;

  if (cam_noise_pulse_width > 0x7FFFFF)
  {
    error_code = FS_ETPU_ERROR_VALUE;
    cam_noise_pulse_width = 0x7FFFFF;
  }
  *(pba + ((FS_ETPU_TOOTHGEN_CAMNOISEPULSEWIDTH_OFFSET - 1)>>2)) = cam_noise_pulse_width;

  return(error_code);
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