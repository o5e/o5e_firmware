/**
 * @file       led_init.c
 * @headerfile led.h
 * @author     sstasiak
 * @brief      led control
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include "led.h"
#if   TRK > 0
#include "led_prv_trk.h"
#elif OEM > 0
#include "led_prv_oem.h"
#else
#error no target hardware defined
#endif

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public
 * @brief init all leds to default state
 * @param none
 * @retval none
 */
void
  led_init( void )
{
  __led_init();
}