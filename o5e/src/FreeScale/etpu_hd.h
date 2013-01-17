/*******************************************************************************
* FILE NAME: etpu_hd.h            COPYRIGHT (c) Freescale Semiconductor 2004  
*                                                All Rights Reserved     
*                                                                        
* DESCRIPTION:                                                           
* This file contains the prototypes and defines for the eTPU Hall Decoder (HD)
* API.                                                  
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
#ifndef _ETPU_HD_H_
#define _ETPU_HD_H_

#include "etpu_hd_auto.h"	/* Auto generated header file for eTPU HD */

/*******************************************************************************
*                            Definitions                                 
*******************************************************************************/

/* revolution_on_off options */
#define FS_ETPU_HD_REV_COUNTING_ON          2  /* revolutions are counted */
#define FS_ETPU_HD_REV_COUNTING_OFF         0  /* revolutions are not counted */

/* direction_on_off options */
#define FS_ETPU_HD_DIRECTION_AUTO_ON        1  /* direction is calculated */
#define FS_ETPU_HD_DIRECTION_AUTO_OFF       0  /* direction is input only */

/* direction options */
#define FS_ETPU_HD_DIRECTION_INC            0  /* initial direction INC */
#define FS_ETPU_HD_DIRECTION_DEC            1  /* initial direction DEC */

/* timer options */
#define FS_ETPU_HD_TCR1                     0  /* TCR1 */
#define FS_ETPU_HD_TCR2                     1  /* TCR2 */

/*===================== Predefined configurations ============================*/
/*============ This is common for all configurations: ========================*/
#define FS_ETPU_HD_A_SECTOR_MASK_LH  0x01
#define FS_ETPU_HD_A_SECTOR_MASK_HL  0x0E
#define FS_ETPU_HD_B_SECTOR_MASK_LH  0x02
#define FS_ETPU_HD_B_SECTOR_MASK_HL  0x0D
#define FS_ETPU_HD_C_SECTOR_MASK_LH  0x04
#define FS_ETPU_HD_C_SECTOR_MASK_HL  0x0B
#define FS_ETPU_HD_D_SECTOR_MASK_LH  0x08
#define FS_ETPU_HD_D_SECTOR_MASK_HL  0x07

/*== One HD phase configuration predefines: ==================================*/
#define FS_ETPU_HD_CFG_0_A_FM_MASK      0x00UL  /* no direction, no rev.count */

/*== Two HD phases configuration predefines: =================================*/
#define FS_ETPU_HD_CFG_1_A_FM_MASK      0x00UL  /* no direction, no rev.count */
#define FS_ETPU_HD_CFG_1_B_FM_MASK      0x00UL  /* no direction, no rev.count */

/*== Three HD phases by 60 degrees configuration predefines: =================*/
#define FS_ETPU_HD_CFG_2_B_DIR_MASK_LH  0x01
#define FS_ETPU_HD_CFG_2_B_DIR_MASK_HL  0x04
#define FS_ETPU_HD_CFG_2_A_FM_MASK      0x00UL  /* no direction, no rev.count */
#define FS_ETPU_HD_CFG_2_B_FM_MASK      0x03UL  /* direction, rev.count */
#define FS_ETPU_HD_CFG_2_C_FM_MASK      0x00UL  /* no direction, no rev.count */

/*== Three HD phases by 120 degrees configuration predefines: ================*/
#define FS_ETPU_HD_CFG_3_A_DIR_MASK_LH  0x04
#define FS_ETPU_HD_CFG_3_A_DIR_MASK_HL  0x02
#define FS_ETPU_HD_CFG_3_B_DIR_MASK_LH  0x01
#define FS_ETPU_HD_CFG_3_B_DIR_MASK_HL  0x04
#define FS_ETPU_HD_CFG_3_C_DIR_MASK_LH  0x02
#define FS_ETPU_HD_CFG_3_C_DIR_MASK_HL  0x01
#define FS_ETPU_HD_CFG_3_A_FM_MASK      0x03UL  /* direction, rev.count */
#define FS_ETPU_HD_CFG_3_B_FM_MASK      0x01UL  /* direction, no rev.count */
#define FS_ETPU_HD_CFG_3_C_FM_MASK      0x01UL  /* direction, no rev.count */
                                                 
/*== Four HD phases by 45 degrees configuration predefines: ==================*/
#define FS_ETPU_HD_CFG_4_B_DIR_MASK_LH  0x01
#define FS_ETPU_HD_CFG_4_B_DIR_MASK_HL  0x04
#define FS_ETPU_HD_CFG_4_C_DIR_MASK_LH  0x02
#define FS_ETPU_HD_CFG_4_C_DIR_MASK_HL  0x08
#define FS_ETPU_HD_CFG_4_A_FM_MASK      0x00UL  /* no direction, no rev.count */
#define FS_ETPU_HD_CFG_4_B_FM_MASK      0x03UL  /* direction, rev.count */
#define FS_ETPU_HD_CFG_4_C_FM_MASK      0x01UL  /* direction, no rev.count */
#define FS_ETPU_HD_CFG_4_D_FM_MASK      0x00UL  /* no direction, no rev.count */

/*== Four HD phases by 90 and 45 degrees configuration predefines: ===========*/
#define FS_ETPU_HD_CFG_5_A_DIR_MASK_LH  0x08
#define FS_ETPU_HD_CFG_5_A_DIR_MASK_HL  0x02
#define FS_ETPU_HD_CFG_5_B_DIR_MASK_LH  0x01
#define FS_ETPU_HD_CFG_5_B_DIR_MASK_HL  0x04
#define FS_ETPU_HD_CFG_5_C_DIR_MASK_LH  0x02
#define FS_ETPU_HD_CFG_5_C_DIR_MASK_HL  0x08
#define FS_ETPU_HD_CFG_5_D_DIR_MASK_LH  0x04
#define FS_ETPU_HD_CFG_5_D_DIR_MASK_HL  0x01
#define FS_ETPU_HD_CFG_5_A_FM_MASK      0x03UL  /* direction, rev.count */
#define FS_ETPU_HD_CFG_5_B_FM_MASK      0x01UL  /* direction, no rev.count */
#define FS_ETPU_HD_CFG_5_C_FM_MASK      0x01UL  /* direction, no rev.count */
#define FS_ETPU_HD_CFG_5_D_FM_MASK      0x01UL  /* direction, no rev.count */

/*******************************************************************************
* Phase commutation commands structure type definition.   
*******************************************************************************/
typedef struct {       /* Commutation command to execute on ...               */    
  uint32_t lh_i_0;     /*  LH transition, on incremental direction, as first  */
  uint32_t lh_i_1;     /*  LH transition, on incremental direction, as second */
  uint32_t lh_d_0;     /*  LH transition, on decremental direction, as first  */
  uint32_t lh_d_1;     /*  LH transition, on decremental direction, as second */
  uint32_t hl_i_0;     /*  HL transition, on incremental direction, as first  */
  uint32_t hl_i_1;     /*  HL transition, on incremental direction, as second */
  uint32_t hl_d_0;     /*  HL transition, on decremental direction, as first  */
  uint32_t hl_d_1;     /*  HL transition, on decremental direction, as second */
} etpu_hd_phase_commut_cmds_t;

/*******************************************************************************
*                         Function prototypes.
*******************************************************************************/
int32_t fs_etpu_hd_init_1ph( uint8_t channel_phaseA,
                             uint8_t priority,
                             uint8_t timer,
                             uint8_t direction,
                             uint8_t PWMMDC_chan,
         etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds);
                 
int32_t fs_etpu_hd_init_2ph( uint8_t channel_phaseA,
                             uint8_t channel_phaseB,
                             uint8_t priority,
                             uint8_t timer,
                             uint8_t direction,
                             uint8_t PWMMDC_chan,
         etpu_hd_phase_commut_cmds_t *phaseA_commut_cmds,
         etpu_hd_phase_commut_cmds_t *phaseB_commut_cmds);                 
                 
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
           etpu_hd_phase_commut_cmds_t *phaseC_commut_cmds);
                 
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
            etpu_hd_phase_commut_cmds_t *phaseC_commut_cmds);
                 
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
           etpu_hd_phase_commut_cmds_t *phaseD_commut_cmds);
                 
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
                etpu_hd_phase_commut_cmds_t *phaseD_commut_cmds);
                 
int32_t fs_etpu_hd_set_commut_on ( uint8_t channel);
int32_t fs_etpu_hd_set_commut_off( uint8_t channel);
int32_t fs_etpu_hd_set_direction( uint8_t channel,
                                  uint8_t direction );
int32_t fs_etpu_hd_set_revolution_counter( uint8_t channel,
                                           int24_t value);

uint32_t fs_etpu_hd_state_struct_addr( uint8_t channel);
uint8_t  fs_etpu_hd_get_sector( uint8_t channel);
uint8_t  fs_etpu_hd_get_direction( uint8_t channel);
int24_t  fs_etpu_hd_get_revolution_counter( uint8_t channel);
uint24_t fs_etpu_hd_get_revolution_period( uint8_t channel);
uint24_t fs_etpu_hd_get_sector_period( uint8_t channel);
uint8_t  fs_etpu_hd_get_pin_state( uint8_t channel);


/*******************************************************************************
*======================== for TPU3 API Compatibility ===========================
*******************************************************************************/

/*******************************************************************************
*                            Definitions                                 
*******************************************************************************/
#define HALLD_TWO_CHANNEL_MODE   0x02
#define HALLD_THREE_CHANNEL_MODE 0x03

#define HALLD_COMMONSTR_CHANNELS_OFFSET   0x08

#define HALLD_DIRECTION_0  0x0000
#define HALLD_DIRECTION_1  0x0001

#define TPU_PRIORITY_HIGH   FS_ETPU_PRIORITY_HIGH
#define TPU_PRIORITY_MIDDLE FS_ETPU_PRIORITY_MIDDLE
#define TPU_PRIORITY_LOW    FS_ETPU_PRIORITY_LOW

/*******************************************************************************
*                         Function prototypes.
*******************************************************************************/

void tpu_halld_init(struct TPU3_tag *tpu,
                              UINT8 channel,
                              INT16 direction,
                              INT16 state_no_address,
                              UINT8 mode);

void tpu_halld_enable(struct TPU3_tag *tpu, UINT8 channel, UINT8 priority);

void tpu_halld_disable(struct TPU3_tag *tpu, UINT8 channel);

void tpu_halld_set_direction(struct TPU3_tag *tpu, UINT8 channel, INT16 direction);

INT16 tpu_halld_get_state_no(struct TPU3_tag *tpu, UINT8 channel);

INT16 tpu_halld_get_pin_state(struct TPU3_tag *tpu, UINT8 channel);

/*******************************************************************************
*        Information for eTPU Graphical Configuration Tool 
*******************************************************************************/
/* full function name: HD - Hall Decoder */ 
/* channel assignment when (fs_etpu_hd_init_1ph: default); load: hd
   phaseA = channel_phaseA
*/
/* channel assignment when (fs_etpu_hd_init_2ph: default); load: hd
   phaseA = channel_phaseA
   phaseB = channel_phaseB
*/
/* channel assignment when (fs_etpu_hd_init_3ph60: default); load: hd
   phaseA = channel_phaseA
   phaseB = channel_phaseB
   phaseC = channel_phaseC
*/
/* channel assignment when (fs_etpu_hd_init_3ph120: default); load: hd
   phaseA = channel_phaseA
   phaseB = channel_phaseB
   phaseC = channel_phaseC
*/
/* channel assignment when (fs_etpu_hd_init_4ph45: default); load: hd
   phaseA = channel_phaseA
   phaseB = channel_phaseB
   phaseC = channel_phaseC
   phaseD = channel_phaseD
*/
/* channel assignment when (fs_etpu_hd_init_4ph90and45: default); load: hd
   phaseA = channel_phaseA
   phaseB = channel_phaseB
   phaseC = channel_phaseC
   phaseD = channel_phaseD
*/
/* load expression hd

var motor_speed { 0..20000 } [ rpm ]
var pole_pairs { 0..20 } [ - ]
var commutation { true(1), false(0) }

base = 100*motor_speed*pole_pairs*2/(60*etpu_clock_freq)
phaseA = if(commutation, 284*base, 56*base)
phaseB = if(commutation, 284*base, 56*base)
phaseC = if(commutation, 284*base, 56*base)
phaseD = if(commutation, 284*base, 56*base)
*/

#endif

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
