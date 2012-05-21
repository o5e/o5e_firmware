/*******************************************************************************
* FILE NAME: etpu_hd.c            COPYRIGHT (c) Freescale Semiconductor 2004
*                                                All Rights Reserved
*
* DESCRIPTION:
* This file contains the eTPU Hall Decoder (HD) API.
*===============================================================================
* REV      AUTHOR      DATE        DESCRIPTION OF CHANGE
* ---   -----------  ----------    ---------------------
* 1.0   V. Philippov   26/Oct/04   Init function split up to 6 separate
*       M. Brejl                   init functions according to config.
*       M. Princ                   of Hall Decoder Sensors.
*                                  Writing parameters approach was changed.
*                                  Added TPU3 API Compatibility Functions.
*                                  Ready for release.
* 2.0   M. Brejl       17/Dec/04   Updated for new build structure.
* 2.1   M. Princ       02/Mar/05   Updated for new HD implementation and new
*                                  order of parameters in common structure.
* 2.2   M. Brejl       22/Mar/05   New HD API state flow:
*       M. Princ                   init -> ...
*******************************************************************************/
#include "etpu_util.h"          /* Utility routines for working with the eTPU */
#include "etpu_hd.h"            /* eTPU HD API */

extern uint32_t fs_etpu_data_ram_start;
extern uint32_t fs_etpu_data_ram_ext;

/*******************************************************************************
* FUNCTION     : fs_etpu_hd_init_1ph
* PURPOSE      : To initialize an eTPU Hall Decoder with Commutation.
*                Configuration with one phase.
* INPUTS NOTES : This function has 6 parameters:
*
*  channel_phaseA     - This is the number of phaseA channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  priority           - This is the priority to assign to the channel.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
*                       FS_ETPU_PRIORITY_LOW.
*  timer              - This is the timer used for period measurements.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_TCR1 or FS_ETPU_HD_TCR2.
*  direction          - This is the initial direction value.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_INC or FS_ETPU_HD_DIRECTION_DEC.
*  PWMMDC_chan        - This is the number of PWMMDC channel. After commutation
*                       the PWMMDC channel is notified to update the PWM phases.
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  *phaseA_commut_cmds- Pointer to Commutation table for phase A
*
* RETURNS NOTES: Error codes which can be returned are: FS_ETPU_ERROR_VALUE,
*                FS_ETPU_ERROR_MALLOC
*******************************************************************************/
int32_t fs_etpu_hd_init_1ph( uint8_t channel_phaseA,
                             uint8_t priority,
                             uint8_t timer,
                             uint8_t direction,
                             uint8_t PWMMDC_chan,
         etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds)

{
   uint32_t * pba_state;         /* base address of common state parameters */
   uint32_t * pbaA;	             /* parameter base address for Phase A. */
   uint16_t common_str_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
   if(((channel_phaseA>31)&&(channel_phaseA<64))||(channel_phaseA>95)||
      (timer>FS_ETPU_TCR2)||
      (direction>FS_ETPU_HD_DIRECTION_DEC)||
      ((PWMMDC_chan>31)&&(PWMMDC_chan<64))||(PWMMDC_chan>95))
   {
      return(FS_ETPU_ERROR_VALUE);
   }
   #endif

   /****************************************
    * PRAM allocation.
    ***************************************/
   if ((pba_state=fs_etpu_malloc(FS_ETPU_HD_COMMON_STR_SIZE))== 0)
   {
        return(FS_ETPU_ERROR_MALLOC);
   }

   common_str_addr = (uint16_t)((uint32_t)pba_state - fs_etpu_data_ram_start);

   /****************************************
    * Write common parameters.
    ***************************************/
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_LASTEDGE_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET - 1)>>2)) = 0;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET) = (uint8_t)direction;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_TIMER_OFFSET) = (uint8_t)timer;

   /****************************************
    * PRAM allocation.
    ***************************************/
   if (((pbaA=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0))
   {
      return(FS_ETPU_ERROR_MALLOC);
   }

   /****************************************
    * Write channel configuration register.
    ***************************************/
   eTPU->CHAN[channel_phaseA].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaA - fs_etpu_data_ram_start) >> 3);

   /****************************************
    * Write FM (function mode) bits.
    ***************************************/
   eTPU->CHAN[channel_phaseA].SCR.R = FS_ETPU_HD_CFG_0_A_FM_MASK;

   /****************************************
    * Write parameters.
    ***************************************/
   *(pbaA + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_HL;
   *(pbaA + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaA + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;

   /****************************************
    * Write commutation command parameters.
    ***************************************/
   if (phaseA_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaA + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseA_commut_cmds, 32);
   }

   /****************************************
    * Write HSR.
    ***************************************/
   eTPU->CHAN[channel_phaseA].HSRR.R = FS_ETPU_HD_HSR_INIT;

   /****************************************
    * Set channel priority and enable.
    ***************************************/
   fs_etpu_enable(channel_phaseA, priority);

   return(0);
}

/*******************************************************************************
* FUNCTION     : fs_etpu_hd_init_2ph
* PURPOSE      : To initialize an eTPU Hall Decoder with Commutation.
*                Configuration with two phases.
* INPUTS NOTES : This function has 8 parameters:
*
*  channel_phaseA     - This is the number of phaseA channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseB     - This is the number of phaseB channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  priority           - This is the priority to assign to the all HD channels.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
*                       FS_ETPU_PRIORITY_LOW.
*  timer              - This is the timer used for period measurements.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_TCR1 or FS_ETPU_HD_TCR2.
*  direction          - This is the initial direction value.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_INC or FS_ETPU_HD_DIRECTION_DEC.
*  PWMMDC_chan        - This is the number of PWMMDC channel. After commutation
*                       the PWMMDC channel is notified to update the PWM phases.
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  *phaseA_commut_cmds- Pointer to Commutation table for phase A
*  *phaseB_commut_cmds- Pointer to Commutation table for phase B
*
* RETURNS NOTES: Error codes which can be returned are: FS_ETPU_ERROR_VALUE,
*                FS_ETPU_ERROR_MALLOC
*******************************************************************************/
int32_t fs_etpu_hd_init_2ph( uint8_t channel_phaseA,
                             uint8_t channel_phaseB,
                             uint8_t priority,
                             uint8_t timer,
                             uint8_t direction,
                             uint8_t PWMMDC_chan,
         etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds,
         etpu_hd_phase_commut_cmds_t *phaseB_commut_cmds)
{
   uint32_t * pba_state;         /* base address of common state parameters */
   uint32_t * pbaA;	             /* parameter base address for Phase A. */
   uint32_t * pbaB;	             /* parameter base address for Phase B. */
   uint16_t common_str_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
   if(((channel_phaseA>31)&&(channel_phaseA<64))||(channel_phaseA>95)||
      ((channel_phaseB>31)&&(channel_phaseB<64))||(channel_phaseB>95)||
      (timer>FS_ETPU_TCR2)||
      (direction>FS_ETPU_HD_DIRECTION_DEC)||
      ((PWMMDC_chan>31)&&(PWMMDC_chan<64))||(PWMMDC_chan>95))
   {
      return(FS_ETPU_ERROR_VALUE);
   }
   #endif

   /****************************************
    * PRAM allocation.
    ***************************************/
   if ((pba_state=fs_etpu_malloc(FS_ETPU_HD_COMMON_STR_SIZE))== 0)
   {
        return(FS_ETPU_ERROR_MALLOC);
   }

   common_str_addr = (uint16_t)((uint32_t)pba_state - fs_etpu_data_ram_start);

   /****************************************
    * Write common parameters.
    ***************************************/
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_LASTEDGE_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET - 1)>>2)) = 0;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET) = (uint8_t)direction;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_TIMER_OFFSET) = (uint8_t)timer;

   /****************************************
    * PRAM allocation.
    ***************************************/
   if (((pbaA=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaB=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0))
   {
     return(FS_ETPU_ERROR_MALLOC);
   }

   /****************************************
    * Write channel configuration registers.
    ***************************************/
   eTPU->CHAN[channel_phaseA].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaA - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseB].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaB - fs_etpu_data_ram_start) >> 3);

   /****************************************
    * Write FM (function mode) bits.
    ***************************************/
   eTPU->CHAN[channel_phaseA].SCR.R = FS_ETPU_HD_CFG_1_A_FM_MASK;
   eTPU->CHAN[channel_phaseB].SCR.R = FS_ETPU_HD_CFG_1_B_FM_MASK;

   /****************************************
    * Write parameters.
    ***************************************/
   *(pbaA + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_HL;
   *(pbaA + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaA + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_HL;
   *(pbaB + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaB + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;

   /****************************************
    * Write commutation command parameters.
    ***************************************/
   if (phaseA_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaA + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseA_commut_cmds, 32);
   }
   if (phaseB_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaB + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseB_commut_cmds, 32);
   }

   /****************************************
    * Write HSR.
    ***************************************/
   eTPU->CHAN[channel_phaseB].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseA].HSRR.R = FS_ETPU_HD_HSR_INIT;

   /****************************************
    * Set channel priority and enable.
    ***************************************/
   fs_etpu_enable(channel_phaseA, priority);
   fs_etpu_enable(channel_phaseB, priority);

   return(0);
}

/*******************************************************************************
* FUNCTION     : fs_etpu_hd_init_3ph60
* PURPOSE      : To initialize an eTPU Hall Decoder with Commutation.
*                Configuration with three phases and sensors by 60 degrees.
* INPUTS NOTES : This function has 12 parameters:
*
*  channel_phaseA     - This is the number of phaseA channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseB     - This is the number of phaseB channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseC     - This is the number of phaseC channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  priority           - This is the priority to assign to the all HD channels.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
*                       FS_ETPU_PRIORITY_LOW.
*  timer              - This is the timer used for period measurements.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_TCR1 or FS_ETPU_HD_TCR2.
*  revolution_on_off  - This parameter enables/disables counting of motor
*                       revolutions.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_REV_COUNTING_ON or
*                       FS_ETPU_HD_REV_COUNTING_OFF
*  direction_on_off   - This parameter enables/disables automatic determination
*                       of motor direction.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_AUTO_ON or
*                       FS_ETPU_HD_DIRECTION_AUTO_OFF
*  direction          - This is the initial direction value.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_INC or FS_ETPU_HD_DIRECTION_DEC.
*  PWMMDC_chan        - This is the number of PWMMDC channel. After commutation
*                       the PWMMDC channel is notified to update the PWM phases.
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  *phaseA_commut_cmds- Pointer to Commutation table for phase A
*  *phaseB_commut_cmds- Pointer to Commutation table for phase B
*  *phaseC_commut_cmds- Pointer to Commutation table for phase C
*
* RETURNS NOTES: Error codes which can be returned are: FS_ETPU_ERROR_VALUE,
*                FS_ETPU_ERROR_MALLOC
*******************************************************************************/
int32_t fs_etpu_hd_init_3ph60( uint8_t channel_phaseA,
                               uint8_t channel_phaseB,
                               uint8_t channel_phaseC,
                               uint8_t priority,
                               uint8_t timer,
                               uint8_t revolution_on_off,
                               uint8_t direction_on_off,
                               uint8_t direction,
                               uint8_t PWMMDC_chan,
           etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds,
           etpu_hd_phase_commut_cmds_t *phaseB_commut_cmds,
           etpu_hd_phase_commut_cmds_t *phaseC_commut_cmds)

{
   uint32_t * pba_state;         /* base address of common state parameters */
   uint32_t * pbaA;	             /* parameter base address for Phase A. */
   uint32_t * pbaB;	             /* parameter base address for Phase B. */
   uint32_t * pbaC;	             /* parameter base address for Phase C. */
   uint16_t common_str_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
   if(((channel_phaseA>31)&&(channel_phaseA<64))||(channel_phaseA>95)||
      ((channel_phaseB>31)&&(channel_phaseB<64))||(channel_phaseB>95)||
      ((channel_phaseC>31)&&(channel_phaseC<64))||(channel_phaseC>95)||
      (timer>FS_ETPU_TCR2)||
      (revolution_on_off>FS_ETPU_HD_REV_COUNTING_ON)||
      (direction_on_off>FS_ETPU_HD_DIRECTION_AUTO_ON)||
      (direction>FS_ETPU_HD_DIRECTION_DEC)||
      ((PWMMDC_chan>31)&&(PWMMDC_chan<64))||(PWMMDC_chan>95))
   {
      return(FS_ETPU_ERROR_VALUE);
   }
   #endif

   /****************************************
    * PRAM allocation.
    ***************************************/
   if ((pba_state=fs_etpu_malloc(FS_ETPU_HD_COMMON_STR_SIZE))== 0)
   {
        return(FS_ETPU_ERROR_MALLOC);
   }

   common_str_addr = (uint16_t)((uint32_t)pba_state - fs_etpu_data_ram_start);

   /****************************************
    * Write common parameters.
    ***************************************/
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_LASTEDGE_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET - 1)>>2)) = 0;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET) = (uint8_t)direction;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_TIMER_OFFSET) = (uint8_t)timer;

   /****************************************
    * PRAM allocation.
    ***************************************/
   if (((pbaA=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaB=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaC=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0))
   {
     return(FS_ETPU_ERROR_MALLOC);
   }

   /****************************************
    * Write channel configuration registers.
    ***************************************/
   eTPU->CHAN[channel_phaseA].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaA - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseB].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaB - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseC].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaC - fs_etpu_data_ram_start) >> 3);

   /****************************************
    * Write FM (function mode) bits.
    ***************************************/
   eTPU->CHAN[channel_phaseA].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_2_A_FM_MASK;
   eTPU->CHAN[channel_phaseB].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_2_B_FM_MASK;
   eTPU->CHAN[channel_phaseC].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_2_C_FM_MASK;

   /****************************************
    * Write parameters.
    ***************************************/
   *(pbaA + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_HL;
   *(pbaA + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaA + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_HL;
   *(pbaB + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaB + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_LH;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_HL;
   *(pbaC + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaC + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;

   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_2_B_DIR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_2_B_DIR_MASK_HL;

   /****************************************
    * Write commutation command parameters.
    ***************************************/
   if (phaseA_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaA + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseA_commut_cmds, 32);
   }
   if (phaseB_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaB + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseB_commut_cmds, 32);
   }
   if (phaseC_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaC + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseC_commut_cmds, 32);
   }

   /****************************************
    * Write HSR.
    ***************************************/
   eTPU->CHAN[channel_phaseA].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseB].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseC].HSRR.R = FS_ETPU_HD_HSR_INIT;

   /****************************************
    * Set channel priority and enable.
    ***************************************/
   fs_etpu_enable(channel_phaseA, priority);
   fs_etpu_enable(channel_phaseB, priority);
   fs_etpu_enable(channel_phaseC, priority);

   return(0);
}

/*******************************************************************************
* FUNCTION     : fs_etpu_hd_init_3ph120
* PURPOSE      : To initialize an eTPU Hall Decoder with Commutation.
*                Configuration with three phases and sensors by 120 degrees.
* INPUTS NOTES : This function has 12 parameters:
*
*  channel_phaseA     - This is the number of phaseA channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseB     - This is the number of phaseB channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseC     - This is the number of phaseC channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  priority           - This is the priority to assign to the all HD channels.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
*                       FS_ETPU_PRIORITY_LOW.
*  timer              - This is the timer used for period measurements.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_TCR1 or FS_ETPU_HD_TCR2.
*  revolution_on_off  - This parameter enables/disables counting of motor
*                       revolutions.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_REV_COUNTING_ON or
*                       FS_ETPU_HD_REV_COUNTING_OFF
*  direction_on_off   - This parameter enables/disables automatic determination
*                       of motor direction.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_AUTO_ON or
*                       FS_ETPU_HD_DIRECTION_AUTO_OFF
*  direction          - This is the initial direction value.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_INC or FS_ETPU_HD_DIRECTION_DEC.
*  PWMMDC_chan        - This is the number of PWMMDC channel. After commutation
*                       the PWMMDC channel is notified to update the PWM phases.
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  *phaseA_commut_cmds- Pointer to Commutation table for phase A
*  *phaseB_commut_cmds- Pointer to Commutation table for phase B
*  *phaseC_commut_cmds- Pointer to Commutation table for phase C
*
* RETURNS NOTES: Error codes which can be returned are: FS_ETPU_ERROR_VALUE,
*                FS_ETPU_ERROR_MALLOC
*******************************************************************************/
int32_t fs_etpu_hd_init_3ph120( uint8_t channel_phaseA,
                                uint8_t channel_phaseB,
                                uint8_t channel_phaseC,
                                uint8_t priority,
                                uint8_t timer,
                                uint8_t revolution_on_off,
                                uint8_t direction_on_off,
                                uint8_t direction,
                                uint8_t PWMMDC_chan,
            etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds,
            etpu_hd_phase_commut_cmds_t *phaseB_commut_cmds,
            etpu_hd_phase_commut_cmds_t *phaseC_commut_cmds)

{
   uint32_t * pba_state;	       /* base address of common state parameters */
   uint32_t * pbaA;	             /* parameter base address for Phase A. */
   uint32_t * pbaB;	             /* parameter base address for Phase B. */
   uint32_t * pbaC;	             /* parameter base address for Phase C. */
   uint16_t common_str_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
   if(((channel_phaseA>31)&&(channel_phaseA<64))||(channel_phaseA>95)||
      ((channel_phaseB>31)&&(channel_phaseB<64))||(channel_phaseB>95)||
      ((channel_phaseC>31)&&(channel_phaseC<64))||(channel_phaseC>95)||
      (timer>FS_ETPU_TCR2)||
      (revolution_on_off>FS_ETPU_HD_REV_COUNTING_ON)||
      (direction_on_off>FS_ETPU_HD_DIRECTION_AUTO_ON)||
      (direction>FS_ETPU_HD_DIRECTION_DEC)||
      ((PWMMDC_chan>31)&&(PWMMDC_chan<64))||(PWMMDC_chan>95))
   {
      return(FS_ETPU_ERROR_VALUE);
   }
   #endif

   /****************************************
    * PRAM allocation.
    ***************************************/
   if ((pba_state=fs_etpu_malloc(FS_ETPU_HD_COMMON_STR_SIZE))== 0)
   {
        return(FS_ETPU_ERROR_MALLOC);
   }

   common_str_addr = (uint16_t)((uint32_t)pba_state - fs_etpu_data_ram_start);

   /****************************************
    * Write common parameters.
    ***************************************/
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_LASTEDGE_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET - 1)>>2)) = 0;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET) = (uint8_t)direction;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_TIMER_OFFSET) = (uint8_t)timer;

   /****************************************
    * PRAM allocation.
    ***************************************/
   if (((pbaA=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaB=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaC=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0))
   {
     return(FS_ETPU_ERROR_MALLOC);
   }

   /****************************************
    * Write channel configuration registers.
    ***************************************/
   eTPU->CHAN[channel_phaseA].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaA - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseB].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaB - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseC].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaC - fs_etpu_data_ram_start) >> 3);

   /****************************************
    * Write FM (function mode) bits.
    ***************************************/
   eTPU->CHAN[channel_phaseA].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_3_A_FM_MASK;
   eTPU->CHAN[channel_phaseB].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_3_B_FM_MASK;
   eTPU->CHAN[channel_phaseC].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_3_C_FM_MASK;

   /****************************************
    * Write parameters.
    ***************************************/
   *(pbaA + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_HL;
   *(pbaA + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaA + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_HL;
   *(pbaB + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaB + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_LH;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_HL;
   *(pbaC + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaC + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;

   *(pbaA + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_3_A_DIR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_3_A_DIR_MASK_HL;
   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_3_B_DIR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_3_B_DIR_MASK_HL;
   *(pbaC + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_3_C_DIR_MASK_LH;
   *(pbaC + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_3_C_DIR_MASK_HL;

   /****************************************
    * Write commutation command parameters.
    ***************************************/
   if (phaseA_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaA + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseA_commut_cmds, 32);
   }
   if (phaseB_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaB + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseB_commut_cmds, 32);
   }
   if (phaseC_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaC + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseC_commut_cmds, 32);
   }

   /****************************************
    * Write HSR.
    ***************************************/
   eTPU->CHAN[channel_phaseA].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseB].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseC].HSRR.R = FS_ETPU_HD_HSR_INIT;

   /****************************************
    * Set channel priority and enable.
    ***************************************/
   fs_etpu_enable(channel_phaseA, priority);
   fs_etpu_enable(channel_phaseB, priority);
   fs_etpu_enable(channel_phaseC, priority);

   return(0);
}

/*******************************************************************************
* FUNCTION     : fs_etpu_hd_init_4ph45
* PURPOSE      : To initialize an eTPU Hall Decoder with Commutation.
*                Configuration with four phases and sensors by 45 degrees.
* INPUTS NOTES : This function has 14 parameters:
*
*  channel_phaseA     - This is the number of phaseA channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseB     - This is the number of phaseB channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseC     - This is the number of phaseC channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseD     - This is the number of phaseD channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  priority           - This is the priority to assign to the all HD channels.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
*                       FS_ETPU_PRIORITY_LOW.
*  timer              - This is the timer used for period measurements.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_TCR1 or FS_ETPU_HD_TCR2.
*  revolution_on_off  - This parameter enables/disables counting of motor
*                       revolutions.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_REV_COUNTING_ON or
*                       FS_ETPU_HD_REV_COUNTING_OFF
*  direction_on_off   - This parameter enables/disables automatic determination
*                       of motor direction.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_AUTO_ON or
*                       FS_ETPU_HD_DIRECTION_AUTO_OFF
*  direction          - This is the initial direction value.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_INC or FS_ETPU_HD_DIRECTION_DEC.
*  PWMMDC_chan        - This is the number of PWMMDC channel. After commutation
*                       the PWMMDC channel is notified to update the PWM phases.
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  *phaseA_commut_cmds- Pointer to Commutation table for phase A
*  *phaseB_commut_cmds- Pointer to Commutation table for phase B
*  *phaseC_commut_cmds- Pointer to Commutation table for phase C
*  *phaseD_commut_cmds- Pointer to Commutation table for phase D
*
* RETURNS NOTES: Error codes which can be returned are: FS_ETPU_ERROR_VALUE,
*                FS_ETPU_ERROR_MALLOC
*******************************************************************************/
int32_t fs_etpu_hd_init_4ph45( uint8_t channel_phaseA,
                               uint8_t channel_phaseB,
                               uint8_t channel_phaseC,
                               uint8_t channel_phaseD,
                               uint8_t priority,
                               uint8_t timer,
                               uint8_t revolution_on_off,
                               uint8_t direction_on_off,
                               uint8_t direction,
                               uint8_t PWMMDC_chan,
           etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds,
           etpu_hd_phase_commut_cmds_t *phaseB_commut_cmds,
           etpu_hd_phase_commut_cmds_t *phaseC_commut_cmds,
           etpu_hd_phase_commut_cmds_t *phaseD_commut_cmds)

{
   uint32_t * pba_state;         /* base address of common state parameters */
   uint32_t * pbaA;	             /* parameter base address for Phase A. */
   uint32_t * pbaB;	             /* parameter base address for Phase B. */
   uint32_t * pbaC;	             /* parameter base address for Phase C. */
   uint32_t * pbaD;	             /* parameter base address for Phase D. */
   uint16_t common_str_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
   if(((channel_phaseA>31)&&(channel_phaseA<64))||(channel_phaseA>95)||
      ((channel_phaseB>31)&&(channel_phaseB<64))||(channel_phaseB>95)||
      ((channel_phaseC>31)&&(channel_phaseC<64))||(channel_phaseC>95)||
      ((channel_phaseD>31)&&(channel_phaseD<64))||(channel_phaseD>95)||
      (timer>FS_ETPU_TCR2)||
      (revolution_on_off>FS_ETPU_HD_REV_COUNTING_ON)||
      (direction_on_off>FS_ETPU_HD_DIRECTION_AUTO_ON)||
      (direction>FS_ETPU_HD_DIRECTION_DEC)||
      ((PWMMDC_chan>31)&&(PWMMDC_chan<64))||(PWMMDC_chan>95))
   {
      return(FS_ETPU_ERROR_VALUE);
   }
   #endif

   /****************************************
    * PRAM allocation.
    ***************************************/
   if ((pba_state=fs_etpu_malloc(FS_ETPU_HD_COMMON_STR_SIZE))== 0)
   {
        return(FS_ETPU_ERROR_MALLOC);
   }

   common_str_addr = (uint16_t)((uint32_t)pba_state - fs_etpu_data_ram_start);

   /****************************************
    * Write common parameters.
    ***************************************/
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_LASTEDGE_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET - 1)>>2)) = 0;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET) = (uint8_t)direction;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_TIMER_OFFSET) = (uint8_t)timer;

   /****************************************
    * PRAM allocation.
    ***************************************/
   if (((pbaA=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaB=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaC=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaD=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0))
   {
     return(FS_ETPU_ERROR_MALLOC);
   }

   /****************************************
    * Write channel configuration registers.
    ***************************************/
   eTPU->CHAN[channel_phaseA].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaA - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseB].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaB - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseC].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaC - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseD].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaD - fs_etpu_data_ram_start) >> 3);

   /****************************************
    * Write FM (function mode) bits.
    ***************************************/
   eTPU->CHAN[channel_phaseA].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_4_A_FM_MASK;
   eTPU->CHAN[channel_phaseB].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_4_B_FM_MASK;
   eTPU->CHAN[channel_phaseC].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_4_C_FM_MASK;
   eTPU->CHAN[channel_phaseD].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_4_D_FM_MASK;

   /****************************************
    * Write parameters.
    ***************************************/
   *(pbaA + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_HL;
   *(pbaA + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaA + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_HL;
   *(pbaB + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaB + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_LH;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_HL;
   *(pbaC + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaC + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaD + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_D_SECTOR_MASK_LH;
   *(pbaD + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_D_SECTOR_MASK_HL;
   *(pbaD + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaD + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;

   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_4_B_DIR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_4_B_DIR_MASK_HL;
   *(pbaC + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_4_C_DIR_MASK_LH;
   *(pbaC + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_4_C_DIR_MASK_HL;

   /****************************************
    * Write commutation command parameters.
    ***************************************/
   if (phaseA_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaA + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseA_commut_cmds, 32);
   }
   if (phaseB_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaB + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseB_commut_cmds, 32);
   }
   if (phaseC_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaC + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseC_commut_cmds, 32);
   }
   if (phaseD_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaD + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseD_commut_cmds, 32);
   }

   /****************************************
    * Write HSR.
    ***************************************/
   eTPU->CHAN[channel_phaseA].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseB].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseC].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseD].HSRR.R = FS_ETPU_HD_HSR_INIT;

   /****************************************
    * Set channel priority and enable.
    ***************************************/
   fs_etpu_enable(channel_phaseA, priority);
   fs_etpu_enable(channel_phaseB, priority);
   fs_etpu_enable(channel_phaseC, priority);
   fs_etpu_enable(channel_phaseD, priority);

   return(0);
}

/*******************************************************************************
* FUNCTION     : fs_etpu_hd_init_4ph90and45
* PURPOSE      : To initialize an eTPU Hall Decoder with Commutation.
*                Configuration with four phases and sensors by 90 and 45 deg.
* INPUTS NOTES : This function has 14 parameters:
*
*  channel_phaseA     - This is the number of phaseA channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseB     - This is the number of phaseB channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseC     - This is the number of phaseC channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  channel_phaseD     - This is the number of phaseD channel
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  priority           - This is the priority to assign to the all HD channels.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_PRIORITY_HIGH, FS_ETPU_PRIORITY_MIDDLE or
*                       FS_ETPU_PRIORITY_LOW.
*  timer              - This is the timer used for period measurements.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_TCR1 or FS_ETPU_HD_TCR2.
*  revolution_on_off  - This parameter enables/disables counting of motor
*                       revolutions.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_REV_COUNTING_ON or
*                       FS_ETPU_HD_REV_COUNTING_OFF
*  direction_on_off   - This parameter enables/disables automatic determination
*                       of motor direction.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_AUTO_ON or
*                       FS_ETPU_HD_DIRECTION_AUTO_OFF
*  direction          - This is the initial direction value.
*                       This parameter should be assigned a value of:
*                       FS_ETPU_HD_DIRECTION_INC or FS_ETPU_HD_DIRECTION_DEC.
*  PWMMDC_chan        - This is the number of PWMMDC channel. After commutation
*                       the PWMMDC channel is notified to update the PWM phases.
*                       0-31 for ETPU_A and 64-96 for ETPU_B.
*  *phaseA_commut_cmds- Pointer to Commutation table for phase A
*  *phaseB_commut_cmds- Pointer to Commutation table for phase B
*  *phaseC_commut_cmds- Pointer to Commutation table for phase C
*  *phaseD_commut_cmds- Pointer to Commutation table for phase D
*
* RETURNS NOTES: Error codes which can be returned are: FS_ETPU_ERROR_VALUE,
*                FS_ETPU_ERROR_MALLOC
*******************************************************************************/
int32_t fs_etpu_hd_init_4ph90and45( uint8_t channel_phaseA,
                                    uint8_t channel_phaseB,
                                    uint8_t channel_phaseC,
                                    uint8_t channel_phaseD,
                                    uint8_t priority,
                                    uint8_t timer,
                                    uint8_t revolution_on_off,
                                    uint8_t direction_on_off,
                                    uint8_t direction,
                                    uint8_t PWMMDC_chan,
                etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds,
                etpu_hd_phase_commut_cmds_t *phaseB_commut_cmds,
                etpu_hd_phase_commut_cmds_t *phaseC_commut_cmds,
                etpu_hd_phase_commut_cmds_t *phaseD_commut_cmds)

{
   uint32_t * pba_state;	       /* base address of common state parameters */
   uint32_t * pbaA;	             /* parameter base address for Phase A. */
   uint32_t * pbaB;	             /* parameter base address for Phase B. */
   uint32_t * pbaC;	             /* parameter base address for Phase C. */
   uint32_t * pbaD;	             /* parameter base address for Phase D. */
   uint16_t common_str_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
   if(((channel_phaseA>31)&&(channel_phaseA<64))||(channel_phaseA>95)||
      ((channel_phaseB>31)&&(channel_phaseB<64))||(channel_phaseB>95)||
      ((channel_phaseC>31)&&(channel_phaseC<64))||(channel_phaseC>95)||
      ((channel_phaseD>31)&&(channel_phaseD<64))||(channel_phaseD>95)||
      (timer>FS_ETPU_TCR2)||
      (revolution_on_off>FS_ETPU_HD_REV_COUNTING_ON)||
      (direction_on_off>FS_ETPU_HD_DIRECTION_AUTO_ON)||
      (direction>FS_ETPU_HD_DIRECTION_DEC)||
      ((PWMMDC_chan>31)&&(PWMMDC_chan<64))||(PWMMDC_chan>95))
   {
      return(FS_ETPU_ERROR_VALUE);
   }
   #endif

   /****************************************
    * PRAM allocation.
    ***************************************/
   if ((pba_state=fs_etpu_malloc(FS_ETPU_HD_COMMON_STR_SIZE))== 0)
   {
        return(FS_ETPU_ERROR_MALLOC);
   }

   common_str_addr = (uint16_t)((uint32_t)pba_state - fs_etpu_data_ram_start);

   /****************************************
    * Write common parameters.
    ***************************************/
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_LASTEDGE_OFFSET - 1)>>2)) = 0;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET - 1)>>2)) = 0xFFFFFF;
   *(pba_state + ((FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET - 1)>>2)) = 0;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET) = (uint8_t)direction;
   *((uint8_t*)pba_state + FS_ETPU_HD_COMMONSTR_TIMER_OFFSET) = (uint8_t)timer;

   /****************************************
    * PRAM allocation.
    ***************************************/
   if (((pbaA=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaB=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaC=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0)||
       ((pbaD=fs_etpu_malloc(FS_ETPU_HD_NUM_PARMS))== 0))
   {
     return(FS_ETPU_ERROR_MALLOC);
   }

   /****************************************
    * Write channel configuration registers.
    ***************************************/
   eTPU->CHAN[channel_phaseA].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaA - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseB].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaB - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseC].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaC - fs_etpu_data_ram_start) >> 3);
   eTPU->CHAN[channel_phaseD].CR.R = (FS_ETPU_HD_TABLE_SELECT << 24) +
                                     (FS_ETPU_HD_FUNCTION_NUMBER << 16) +
                                     (((uint32_t)pbaD - fs_etpu_data_ram_start) >> 3);

   /****************************************
    * Write FM (function mode) bits.
    ***************************************/
   eTPU->CHAN[channel_phaseA].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_5_A_FM_MASK;
   eTPU->CHAN[channel_phaseB].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_5_B_FM_MASK;
   eTPU->CHAN[channel_phaseC].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_5_C_FM_MASK;
   eTPU->CHAN[channel_phaseD].SCR.R = (direction_on_off + revolution_on_off) & FS_ETPU_HD_CFG_5_D_FM_MASK;

   /****************************************
    * Write parameters.
    ***************************************/
   *(pbaA + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_A_SECTOR_MASK_HL;
   *(pbaA + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaA + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_B_SECTOR_MASK_HL;
   *(pbaB + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaB + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_LH;
   *(pbaC + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_C_SECTOR_MASK_HL;
   *(pbaC + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaC + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;
   *(pbaD + ((FS_ETPU_HD_SECTORMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_D_SECTOR_MASK_LH;
   *(pbaD + ((FS_ETPU_HD_SECTORMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_D_SECTOR_MASK_HL;
   *(pbaD + ((FS_ETPU_HD_COMMONSTR_OFFSET - 1)>>2)) = common_str_addr;
   *((uint8_t*)pbaD + FS_ETPU_HD_PWMMCHAN_OFFSET) = PWMMDC_chan;

   *(pbaA + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_A_DIR_MASK_LH;
   *(pbaA + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_A_DIR_MASK_HL;
   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_B_DIR_MASK_LH;
   *(pbaB + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_B_DIR_MASK_HL;
   *(pbaC + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_C_DIR_MASK_LH;
   *(pbaC + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_C_DIR_MASK_HL;
   *(pbaD + ((FS_ETPU_HD_DIRECTIONMASKLH_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_D_DIR_MASK_LH;
   *(pbaD + ((FS_ETPU_HD_DIRECTIONMASKHL_OFFSET - 1)>>2)) = FS_ETPU_HD_CFG_5_D_DIR_MASK_HL;

   /****************************************
    * Write commutation command parameters.
    ***************************************/
   if (phaseA_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaA + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseA_commut_cmds, 32);
   }
   if (phaseB_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaB + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseB_commut_cmds, 32);
   }
   if (phaseC_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaC + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseC_commut_cmds, 32);
   }
   if (phaseD_commut_cmds)
   {
      fs_memcpy32((uint32_t *)((uint8_t *)pbaD + FS_ETPU_HD_COMMCMDLHI0_OFFSET),
                  (uint32_t *)phaseD_commut_cmds, 32);
   }

   /****************************************
    * Write HSR.
    ***************************************/
   eTPU->CHAN[channel_phaseA].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseB].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseC].HSRR.R = FS_ETPU_HD_HSR_INIT;
   eTPU->CHAN[channel_phaseD].HSRR.R = FS_ETPU_HD_HSR_INIT;

   /****************************************
    * Set channel priority and enable.
    ***************************************/
   fs_etpu_enable(channel_phaseA, priority);
   fs_etpu_enable(channel_phaseB, priority);
   fs_etpu_enable(channel_phaseC, priority);
   fs_etpu_enable(channel_phaseD, priority);

   return(0);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_set_commut_on
*PURPOSE      : To enable commutation processing on one HD channel.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: Error code which can be returned is: FS_ETPU_ERROR_VALUE
*******************************************************************************/
int32_t fs_etpu_hd_set_commut_on ( uint8_t channel)
{
   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
       if(((channel>31)&&(channel<64))||(channel>95))
       {
          return(FS_ETPU_ERROR_VALUE);
       }
   #endif

   fs_etpu_set_hsr(channel, FS_ETPU_HD_HSR_COMMUT_ON);

   return(0);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_set_commut_off
*PURPOSE      : To disable commutation processing on one HD channel.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: Error code which can be returned is: FS_ETPU_ERROR_VALUE
*******************************************************************************/
int32_t fs_etpu_hd_set_commut_off( uint8_t channel)
{
   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
       if(((channel>31)&&(channel<64))||(channel>95))
       {
          return(FS_ETPU_ERROR_VALUE);
       }
   #endif

   fs_etpu_set_hsr(channel, FS_ETPU_HD_HSR_COMMUT_OFF);

   return(0);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_set_direction
*PURPOSE      : To set direction value.
*INPUTS NOTES : This function has 2 parameters:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*               direction- Direction value. This parameter should be assigned
*                          a value of:
*                          FS_ETPU_HD_DIRECTION_INC or
*                          FS_ETPU_HD_DIRECTION_DEC
*RETURNS NOTES: Error code which can be returned is: FS_ETPU_ERROR_VALUE
*******************************************************************************/
int32_t fs_etpu_hd_set_direction( uint8_t channel,
                                  uint8_t direction )
{
   uint32_t hd_state_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
       if(((channel>31)&&(channel<64))||(channel>95)||
          (direction>FS_ETPU_HD_DIRECTION_DEC))
       {
          return(FS_ETPU_ERROR_VALUE);
       }
   #endif

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   *(uint8_t *)((uint32_t)fs_etpu_data_ram_start + hd_state_addr + 
                          FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET) = direction;

   return(0);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_set_revolution_counter
*PURPOSE      : To changes the Revolution Counter value.
*INPUTS NOTES : This function has 2 parameters:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*               value    - Revolution Counter value to be set.
*RETURNS NOTES: Error code which can be returned is: FS_ETPU_ERROR_VALUE
*******************************************************************************/
int32_t fs_etpu_hd_set_revolution_counter( uint8_t channel,
                                           int24_t value  )
{
   uint32_t hd_state_addr;

   /****************************************
    * Parameters bounds check.
    ***************************************/
   #ifdef FS_ETPU_MC_PARAM_CHECK
       if(((channel>31)&&(channel<64))||(channel>95))
       {
          return(FS_ETPU_ERROR_VALUE);
       }
   #endif

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
    *(int24_t *)((uint32_t)fs_etpu_data_ram_start + hd_state_addr + 
                           FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET-1) = value;

   return(0);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_state_struct_addr
*PURPOSE      : To get address of the state structure beginning
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: This function returns the addres
*               of the state structure beginning.
*******************************************************************************/
uint32_t fs_etpu_hd_state_struct_addr( uint8_t channel)
{
   uint32_t hd_state_addr;

   hd_state_addr = fs_etpu_data_ram_start + 
                fs_etpu_get_chan_local_24(channel, FS_ETPU_HD_COMMONSTR_OFFSET);

   return(hd_state_addr);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_get_sector
*PURPOSE      : To get sector value.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: This function returns sector value
*******************************************************************************/
uint8_t fs_etpu_hd_get_sector( uint8_t channel)
{
   uint8_t  sector;
   uint32_t hd_state_addr;

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   sector = (uint8_t)(fs_etpu_get_global_24(hd_state_addr + 
                                           FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET));

   return(sector);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_get_direction
*PURPOSE      : To get direction value.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: This function returns direction value. The returned value
*               can be one of:
*               FS_ETPU_HD_DIRECTION_INC
*               FS_ETPU_HD_DIRECTION_DEC
*******************************************************************************/
uint8_t fs_etpu_hd_get_direction( uint8_t channel)
{
   uint8_t direction;
   uint32_t hd_state_addr;

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   direction = fs_etpu_get_global_8(hd_state_addr + 
                                    FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET);

   if (direction == 0)
   {
      direction = FS_ETPU_HD_DIRECTION_INC;
   }
   else
   {
      direction = FS_ETPU_HD_DIRECTION_DEC;
   }

   return(direction);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_get_revolution_counter
*PURPOSE      : To get revolution counter value.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: This function returns revolution counter value
*******************************************************************************/
int24_t fs_etpu_hd_get_revolution_counter( uint8_t channel)
{
   int24_t rev_counter;
   uint32_t hd_state_addr;

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   rev_counter = fs_etpu_get_global_24s(hd_state_addr + 
                                        FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET);

   return(rev_counter);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_get_revolution_period
*PURPOSE      : To get revolution period value.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: This function returns revolution period value
*******************************************************************************/
uint24_t fs_etpu_hd_get_revolution_period( uint8_t channel)
{
   uint24_t rev_period;
   uint32_t hd_state_addr;

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   rev_period = fs_etpu_get_global_24(hd_state_addr + 
                                      FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET);

   return(rev_period);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_get_sector_period
*PURPOSE      : To get sector period value.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: This function returns sector period value
*******************************************************************************/
uint24_t fs_etpu_hd_get_sector_period( uint8_t channel)
{
   uint24_t sector_period;
   uint32_t hd_state_addr;

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   sector_period = fs_etpu_get_global_24(hd_state_addr + 
                                      FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET);

   return(sector_period);
}

/*******************************************************************************
*FUNCTION     : fs_etpu_hd_get_pin_state
*PURPOSE      : To get selected HD channel pin state.
*INPUTS NOTES : This function has 1 parameter:
*               channel  - This is the HD channel number.
*                          0-31 for ETPU_A and 64-96 for ETPU_B.
*RETURNS NOTES: This function returns selected HD channel pin state.
*******************************************************************************/
uint8_t fs_etpu_hd_get_pin_state( uint8_t channel)
{
   uint8_t pin_state;

   pin_state = eTPU->CHAN[channel].SCR.B.IPS;

   return(pin_state);
}



/*******************************************************************************
*======================= TPU3 API Compatibility Functions ======================
*******************************************************************************/
/*******************************************************************************
FUNCTION      : tpu_halld_init
PURPOSE       : To initialize two or three TPU channels to run the HALLD
                function.
INPUTS NOTES  : This function has 5 parameters:

                  *tpu - This is a pointer to the TPU3 module. Not used.

               channel - This is the channel number of the primary HALLD
                         channel. The following channels is used for two channel
                         mode and following two channels are used for three
                         channel mode.

             direction - This value defines the DIRECTION value in the parameter
                         RAM. This parameter should be assigned a value of:
											 HALLD_DIRECTION_0;
											 HALLD_DIRECTION_1.

      state_no_address - This parameter is not used.

               mode    - This parameter selects either two or three channel mode
                         This parameter should be assigned a value of:
											 HALLD_TWO_CHANNEL_MODE;
											 HALLD_THREE_CHANNEL_MODE.
RETURNS NOTES : none
*******************************************************************************/
void tpu_halld_init(struct TPU3_tag *tpu, UINT8 channel, INT16 direction,
                    INT16 state_no_address, UINT8 mode)
{
   uint32_t hd_state_addr;

   if (mode==HALLD_TWO_CHANNEL_MODE)
   {
      fs_etpu_hd_init_2ph( channel,
                           (uint8_t)(channel+1),
                           FS_ETPU_PRIORITY_DISABLE,
                           FS_ETPU_TCR1,
                           (uint8_t)direction,
                           31,
                           0,
                           0);

      /* Get address of the HD common structure beginning */
      hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);

      /* Save number of initialized HD channels into HD COMMON STRUCTURE (offset = 8). */
      *(uint8_t *)((uint32_t)fs_etpu_data_ram_start + hd_state_addr + HALLD_COMMONSTR_CHANNELS_OFFSET) = 2;
   }
   else
   {
      fs_etpu_hd_init_3ph60( channel,
                             (uint8_t)(channel+1),
                             (uint8_t)(channel+2),
                             FS_ETPU_PRIORITY_DISABLE,
                             FS_ETPU_TCR1,
                             FS_ETPU_HD_REV_COUNTING_OFF,
                             FS_ETPU_HD_DIRECTION_AUTO_OFF,
                             (uint8_t)direction,
                             31,
                             0,
                             0,
                             0);

      /* Get address of the HD common structure beginning */
      hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);

      /* Save number of initialized HD channels into HD COMMON STRUCTURE (offset = 8). */
      *(uint8_t *)((uint32_t)fs_etpu_data_ram_start + hd_state_addr + HALLD_COMMONSTR_CHANNELS_OFFSET) = 3;
   }
}

/*******************************************************************************
FUNCTION      : tpu_halld_enable
PURPOSE       : To enable intialised TPU channels to run the HALLD function
INPUTS NOTES  : This function has 3 parameters:

                 *tpu - This is a pointer to the TPU3 module. Not used.

              channel - This is the channel number of the primary HALLD
                        channel.

             priority - This parameter should be assigned a value of:
                        TPU_PRIORITY_HIGH, TPU_PRIORITY_MIDDLE or
                        TPU_PRIORITY_LOW. All channels are assigned the same
                        priority. Although this is not necessary for correct
                        operation it simplifies system performance calculations
RETURNS NOTES : none
*******************************************************************************/
void tpu_halld_enable(struct TPU3_tag *tpu, UINT8 channel, UINT8 priority)
{
   uint8_t channels;
   uint32_t hd_state_addr;

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   channels = fs_etpu_get_global_8(hd_state_addr + HALLD_COMMONSTR_CHANNELS_OFFSET);

   fs_etpu_enable( channel, priority);
   fs_etpu_enable( (uint8_t)(channel+1), priority);
   if (channels == HALLD_THREE_CHANNEL_MODE )
   {
      fs_etpu_enable( (uint8_t)(channel+2), priority);
   }
}

/*******************************************************************************
FUNCTION      : tpu_halld_disable
PURPOSE       : To disable TPU channels setup to run the HALLD function
INPUTS NOTES  : This function has 2 parameters:

                    *tpu - This is a pointer to the TPU3 module. Not used.

                 channel - This is the channel number of the primary HALLD
                           channel.
RETURNS NOTES : none
*******************************************************************************/
void tpu_halld_disable(struct TPU3_tag *tpu, UINT8 channel)
{
   uint8_t channels;
   uint32_t hd_state_addr;

   hd_state_addr = 0x00007FFF & fs_etpu_hd_state_struct_addr(channel);
   channels = fs_etpu_get_global_8(hd_state_addr + HALLD_COMMONSTR_CHANNELS_OFFSET);

   fs_etpu_disable(channel);
   fs_etpu_disable(channel+1);
   if (channels == 3)
   {
      fs_etpu_disable(channel+2);
   }
}

/*******************************************************************************
FUNCTION      : tpu_halld_set_direction
PURPOSE       : To set the DIRECTION parameter in the HALLD function paramter
                RAM.
INPUTS NOTES  : This function has 3 parameters:

               *tpu - This is a pointer to the TPU3 module. Not used.

            channel - This is the channel number of the channel A HALLD
                      channel. The following channel is used for two channel
                      mode and following two channels are used for three
                      channel mode.

          direction - This value defines the DIRECTION value in the parameter
                      RAM. It has the value 0x0000 or 0x0001.
RETURNS NOTES : none
*******************************************************************************/
void tpu_halld_set_direction(struct TPU3_tag *tpu, UINT8 channel, INT16 direction)
{
   	 fs_etpu_hd_set_direction(channel,(uint8_t) direction);
   	 return;
}

/*******************************************************************************
FUNCTION      : tpu_halld_get_state_no
PURPOSE       : returns the STATE_NO (Sector)
INPUTS NOTES  : This function has 2 parameters:

                 *tpu - This is a pointer to the TPU3 module. Not used.

                 channel - This is the channel number of the HALLD function
                           channel A.

RETURNS NOTES : returns STATE_NO value as INT16
*******************************************************************************/
INT16 tpu_halld_get_state_no(struct TPU3_tag *tpu, UINT8 channel)
{
   return((INT16)fs_etpu_hd_get_sector( channel));
}

/*******************************************************************************
FUNCTION      : tpu_halld_get_pinstate
PURPOSE       : returns the PINSTATE parameter from the TPU HALLD function
                parameter RAM
INPUTS NOTES  : This function has 2 parameters:

    *tpu - This is a pointer to the TPU3 module. Not used.

    channel - This is the channel number of the HALLD function channel from
    	   whcih pinstate is to be returned. This can be anyone of the
    	   two/three HALLD channels.

RETURNS NOTES : returns PINSTATE value as INT16
*******************************************************************************/
INT16 tpu_halld_get_pin_state(struct TPU3_tag *tpu, UINT8 channel)
{
   return (INT16) fs_etpu_hd_get_pin_state(channel);
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
