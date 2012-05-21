
#include "config.h"

#include "system.h"
#include "FSutil.h"
#include "mpc5500_usrccdcfg.h"

/**************************************************************************/
/* FILE NAME: mpc5500_demo.c                 COPYRIGHT (c) Freescale 2004 */
/*                                                All Rights Reserved     */
/* DESCRIPTION:                                                           */
/* This file contains functions for the MPC5500 to initialize and turn on */
/*  an LED. This provides external proof that the program reached main( ).*/
/*  This file will be replaced by the user application code.              */
/*========================================================================*/
/* REV      AUTHOR       DATE       DESCRIPTION OF CHANGE                 */
/* ---   -----------   ----------   ---------------------                 */
/* 0.1   G. Jackson    26/Apr/04     Initial version                      */
/* 0.2   G. Jackson    30/Jul/04     Added functions to change one or two */
/*                                     outputs at a time.                 */
/* 0.3   C. Baker      19/Jul/06     Moved #includes to                   */
/*                                     mpc5500_usrccdcfg.h, removed       */
/*                                     unused functions init_LED2 and     */
/*                                     invert_LED2                        */
/**************************************************************************/



/**************************************************************************/
/*                       C Code Functions                                 */
/**************************************************************************/

/************************************************************************/
/* FUNCTION     : init_LED                                              */
/* PURPOSE      : This function intializes one GPIO port for output.    */
/* INPUT NOTES  : init_pin -- GPIO pin used as output.                  */
/*                pinout_val -- initialize LED on (1) or off (0).       */
/* RETURN NOTES : None                                                  */
/* WARNING      : None                                                  */
/************************************************************************/
void init_LED(uint8_t init_pin, uint8_t pinout_val)
{

/* configure GPIO pin 1 for output using gpio_config function. */
    mc_gpio_config( init_pin, GPIO_FUNCTION | OUTPUT_MODE | DRIVE_STRENGTH_50PF );

/* set LED to one */
  if(pinout_val == 1){
      mc_gpio_output(init_pin,PIN_HI);
    }
/* set LED to zero */
  else {
      mc_gpio_output(init_pin,PIN_LO);
    }
}

/***********************************************************************/
/* FUNCTION     : invert_LED                                           */
/* PURPOSE      : Inverts one GPIO output.                             */
/* INPUT NOTES  : inv_pin - GPIO pin to be inverted.                   */
/* RETURN NOTES : None                                                 */
/* WARNING      : None                                                 */
/***********************************************************************/

void invert_LED(uint8_t inv_pin)
{
    SIU.GPDO[inv_pin].R = !SIU.GPDO[inv_pin].R; /* Invert gpio port */
} 

/******************************************************************************/
/* FUNCTION     : mc_gpio_config                                              */
/* PURPOSE      : This function configures a port.                            */
/* INPUTS NOTES : This function has 2 parameters:                             */
/*                port - This is the port number. The port number should      */
/*                  be between 0-511. It will typically be less for any       */
/*                  given device. The port numbers may not be continuous.     */
/*                config - This is the confiuration value for the port.       */
/*                  The best way to determine this is to add the              */
/*                  configuration values together from the gpio.h file.       */
/* RETURNS NOTES: none.                                                       */
/* WARNING      :                                                             */
/******************************************************************************/
void mc_gpio_config( uint16_t port, uint16_t config){
    SIU.PCR[port].R = config;
}

/*****************************************************************************/
/* FUNCTION     : mc_gpio_output                                             */
/* PURPOSE      : This function sets the state of a port.                    */
/* INPUTS NOTES : This function has 2 parameters:                            */
/*                port - This is the port number. The port number should     */
/*                  be between 0-511. It will typically be less for any      */ 
/*                  given device. The port numbers may not be continuous.    */
/*                value - Sets the output level of the GPIO port.            */ 
/* RETURN NOTES: none.                                                       */
/* WARNING      : This function assumes that the port is allready            */
/*                    configured for output.                                 */
/*****************************************************************************/
void mc_gpio_output( uint16_t port, uint8_t value) {
   SIU.GPDO[port].R = value;
}

