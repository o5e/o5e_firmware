/**************************************************************************
 * FILE NAME: $RCSfile: etpu_gpio.c,v $      COPYRIGHT (c) FREESCALE 2004 *
 * DESCRIPTION:                                     All Rights Reserved   *
 * This file contains  ETPU General Purpose Input/Output (GPIO) API       *
 *========================================================================*
 * ORIGINAL AUTHOR: Ken Terry (ttz102)                                    *
 * $Log: etpu_gpio.c,v $
 * Revision 2.0  2004/12/08 11:46:15  r47354
 * no message
 *
 * Revision 1.1  2004/12/08 11:45:09  r47354
 * Updates as per QOM API rel_2_1
 *
 *........................................................................*
 * 0.1   K Terry      12/May/04  Initial version.                         *
 * 0.2   K Terry      03/Jun/04  Mod'd to use fs_etpu_util functions      *
 * 0.3   K Terry      21/Jun/04  Initialisation function mod'd so that    *
 *                           channel is disabled prior to initilisation   *
 * 0.4                           Updated for new build structure.         *
 * 0.5   K Terry       8/Jul/04  Initialisation function mod'd to return  *
 *                               0 on successful initialisation           *
 * 0.6   G.Emerson   15/Jul/04   Updated FM bits for MPC5554.h change     *
 *                               Add disclaimer                           *
 **************************************************************************/
#include "etpu_util.h"          /* Utility routines for working with the eTPU */
#include "etpu_gpio.h"          /* eTPU GPIO API */
extern uint32_t fs_etpu_data_ram_start;

/******************************************************************************
FUNCTION     : etpu_gpio_init
PURPOSE      : To initialize an eTPU channel to for General Purpose I/O.
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
               priority - This is the priority to assign to the channel.
                          This parameter should be assigned a value of:
                          ETPU_PRIORITY_HIGH, ETPU_PRIORITY_MIDDLE or
                          ETPU_PRIORITY_LOW.
WARNING      :*This function does not configure the pin only the eTPU. In a
                  system a pin may need to be configured to select the eTPU.
              *This function requires that the system frequencies are defined
                  external to this function.
******************************************************************************/
int32_t fs_etpu_gpio_init (uint8_t channel, uint8_t priority)
{
    uint32_t *pba;              /* parameter base address for channel */

    /* Disable channel to assign function safely */
    fs_etpu_disable (channel);

    /* get parameter RAM */
    /* number of parameters passed from eTPU C code */
    pba = fs_etpu_malloc (FS_ETPU_GPIO_NUM_PARMS);

    if (pba == 0)
        return (FS_ETPU_ERROR_MALLOC);

    /* write chan config register -  
       ETPU_GPIO_TABLE_SELECT and ETPU_GPIO_FUNCTION_NUMBER
       supplied from etpu_gpio_auto.h */
    eTPU->CHAN[channel].CR.R =
        (priority << 28) + (FS_ETPU_GPIO_TABLE_SELECT << 24) +
        (FS_ETPU_GPIO_FUNCTION_NUMBER << 16) + (((uint32_t) pba -
         fs_etpu_data_ram_start) >> 3);

    *(pba + ((FS_ETPU_GPIO_PINSTATE_OFFSET - 1) >> 2)) = 0;
    /*initialise pinstate = 0 */
    return (0);
};

/******************************************************************************
FUNCTION     : etpu_gpio_output_high
PURPOSE      : Sets GPIO channel output high.
INPUTS NOTES : This function has 1 parameter:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
RETURN NOTES : None
******************************************************************************/
void fs_etpu_gpio_output_high (uint8_t channel)
{

/* write hsr */
    eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_OUTPUT_HIGH;
    /* ETPU_GPIO_OUTPUT_HIGH supplied from etpu_gpio_auto.h */
};

/******************************************************************************
FUNCTION     : etpu_gpio_output_low
PURPOSE      : Sets GPIO channel output low.
INPUTS NOTES : This function has 1 parameter:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
RETURN NOTES : None
******************************************************************************/
void fs_etpu_gpio_output_low (uint8_t channel)
{

/* write hsr */
    eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_OUTPUT_LOW;
    /* ETPU_GPIO_OUTPUT_LOW supplied from etpu_gpio_auto.h */
};

/******************************************************************************
FUNCTION     : etpu_gpio_output
PURPOSE      : Sets GPIO channel output low.
INPUTS NOTES : This function has 2 parameter:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
               level - this is the level to be output 
               0 for ETPU_OP_LOW or ETPU_OP_HIGH
RETURN NOTES : None
******************************************************************************/
void fs_etpu_gpio_output (uint8_t channel, uint8_t level)
{

    if (level == FS_ETPU_OP_HIGH)
      {
          /* write hsr */
          eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_OUTPUT_HIGH;
          /*ETPU_GPIO_OUTPUT_HIGH supplied from etpu_gpio_auto.h */
      }
    else
      {
          /* write hsr */
          eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_OUTPUT_LOW;
          /* ETPU_GPIO_OUTPUT_LOW supplied from etpu_gpio_auto.h */
      }
};

/******************************************************************************
FUNCTION     : etpu_gpio_cfg_input_trans
PURPOSE      : Configures eTPU GPIO channel for input transition mode - 
               update on transition
	             The lsb of eTPU parameter PINSTATE is updated with a value 
	             determined by the edge occuring on the associate eTPU channel
	             pin (1 for rising , 0 for falling) and PINSTATE is rotated left
	             by one bit.
INPUTS NOTES : This function has 2 parameters:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
               mode - determines the edge on which a transition is to occur
               ETPU_GPIO_IP_RISING, ETPU_GPIO_IP_FALLING, ETPU_GPIO_IP_EITHER.
RETURN NOTES : None
******************************************************************************/
void fs_etpu_gpio_cfg_input_trans (uint8_t channel, uint8_t mode)
{

    if (mode == FS_ETPU_GPIO_IP_RISING)
      {
          /* write hsr */
          eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_INPUT_RISING;
          /* ETPU_GPIO_INPUT_RISING supplied from etpu_gpio_auto.h */
      }
    else if (mode == FS_ETPU_GPIO_IP_FALLING)
      {
          /* write hsr */
          eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_INPUT_FALLING;
          /* ETPU_GPIO_INPUT_FALLING supplied from etpu_gpio_auto.h */
      }
    else
      {
          /* write hsr */
          eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_INPUT_EITHER;
          /* ETPU_GPIO_INPUT_EITHER supplied from etpu_gpio_auto.h */
      }

};

/******************************************************************************
FUNCTION     : etpu_gpio_cfg_input_periodic
PURPOSE      : Configures eTPU GPIO channel for input transition mode - 
               update on transition
	             The lsb of eTPU parameter PINSTATE is updated with a value 
	             determined by the logic level on the associated eTPU channel 
	             pin and PINSTATE is rotated left by one bit pin at a rate 
	             defined by function input parameter rate.
INPUTS NOTES : This function has 3 parameters:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
               tcr - determines whether TCR1 or TCR2 is used as timebase - 
               this should be assigned a value of ETPU_TCR1 or ETPU_TCR2
               rate - This is a 32 bit value, the lower 24 bits of which, 
               are written to the selected timer counter register to define 
               the update period.
RETURN NOTES : None
******************************************************************************/
void fs_etpu_gpio_cfg_input_periodic (uint8_t channel, uint8_t timebase,
                                 uint32_t rate)
{
    uint32_t *pba;              /*parameter base address for channel */

    pba = fs_etpu_data_ram (channel);
    pba = pba + ((FS_ETPU_GPIO_INPUT_MATCH_PERIOD_OFFSET - 1) >> 2);
    /*pba points to InputMatchPeriod in Parameter RAM */
    *pba = rate;                /* InputMatchPeriod = rate */

    if (timebase == FS_ETPU_TCR1)
        eTPU->CHAN[channel].SCR.B.FM0 = 0x0;    /* Function Mode bit = 0 to select TCR1 */
    else
        eTPU->CHAN[channel].SCR.B.FM0 = 0x1;    /* Function Mode bit = 1 to select TCR2 */

    /* write hsr */
    eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_INPUT_MATCH;
    /* ETPU_GPIO_INPUT_MATCH supplied from etpu_gpio_auto.h */

};

/******************************************************************************
FUNCTION     : etpu_gpio_input_immed
PURPOSE      : input immediate input mode - updates PINSTATE immediately 
               according to input pin level
	             The lsb of eTPU parameter PINSTATE is updated with a value 
	             determined by the logic level on the associated eTPU channel
	             pin and PINSTATE is rotated left by one bit pin at a rate defined 
	             by function input parameter rate.
INPUTS NOTES : This function has 1 parameter:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
RETURN NOTES : None
******************************************************************************/
void fs_etpu_gpio_input_immed (uint8_t channel)
{
    uint32_t *pba;              /* parameter base address for channel */
    uint32_t pinstate;
    /* write hsr */
    eTPU->CHAN[channel].HSRR.R = FS_ETPU_GPIO_INPUT_IMMED;
    /* ETPU_GPIO_INPUT_IMMED supplied from etpu_gpio_auto.h */

};

/******************************************************************************
FUNCTION     : etpu_gpio_pin_history
PURPOSE      : etpu_gpio_pin_history - returns PINSTATE
INPUTS NOTES : This function has 1 parameter:
               channel - This is the channel number.
                           0-31 for ETPU_A and 64-95 for ETPU_B.
RETURN NOTES : Returns a 32 bit uint value containing PINSTATE in the 
               lower 24 bits
******************************************************************************/
uint32_t fs_etpu_gpio_pin_history (uint8_t channel)
{
    uint32_t *pba;              /* parameter base address for channel */
    uint32_t pinstate;

    pba = fs_etpu_data_ram (channel);
    pba = pba + ((FS_ETPU_GPIO_PINSTATE_OFFSET - 1) >> 2);
    /*pba points to PINSTATE in Parameter RAM */
    pinstate = *pba & 0x00ffffff;
    /* mask bits 25 - 31 - pinstate is 24 bit value in parameter RAM */

    return (pinstate);

};

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
