/**************************************************************************
 * FILE NAME: $RCSfile: etpu_gpio.h,v $      COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains the prototypes and defines for the ETPU General     *
 * Purpose Input/Output (GPIO) API                                        *
 *========================================================================*
 * ORIGINAL AUTHOR: Ken Terry (ttz102)                                    *
 * $Log: etpu_gpio.h,v $
 * Revision 1.1  2004/12/08 11:45:09  r47354
 * Updates as per QOM API rel_2_1
 *
 *........................................................................*
/* 0.1   K Terry     29/Apr/03      Initial version.                      */
/* 0.2                              Updated for new build structure.      */
/* 0.3   G.Emerson   16/July/04     Add disclaimer                        */
/**************************************************************************/
#include "etpu_gpio_auto.h"	/* Auto generated header file for eTPU GPIO */

/**************************************************************************/
/*                            Definitions                                 */
/**************************************************************************/
/**************************************************************************/
/*                            Definitions                                 */
/**************************************************************************/
#define FS_ETPU_OP_HIGH 1
#define FS_ETPU_OP_LOW 0
#define FS_ETPU_GPIO_IP_RISING 0
#define FS_ETPU_GPIO_IP_FALLING 1
#define FS_ETPU_GPIO_IP_EITHER 2



/**************************************************************************/
/*                       Function Prototypes                              */
/**************************************************************************/

/* GPIO channel initialisation */
int32_t fs_etpu_gpio_init( uint8_t channel, uint8_t priority);

/* GPIO output logic high */
void fs_etpu_gpio_output_high (uint8_t channel);

/* GPIO output logic low */
void fs_etpu_gpio_output_low (uint8_t channel);

/* GPIO set otuput according to parameter level */
void fs_etpu_gpio_output (uint8_t channel, uint8_t level);

/* Configure channel for input transition mode */
void fs_etpu_gpio_cfg_input_trans (uint8_t channel, uint8_t mode);

/* Configure GPIO channel for input periodic mode */
void fs_etpu_gpio_cfg_input_periodic (uint8_t channel, uint8_t timebase, uint32_t rate);

/* update eTPU parameter PINSTATE immediately */
void fs_etpu_gpio_input_immed (uint8_t channel);

/* return eTPU parameter PINSTATE */
uint32_t fs_etpu_gpio_pin_history (uint8_t channel);


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
 *
 ********************************************************************/