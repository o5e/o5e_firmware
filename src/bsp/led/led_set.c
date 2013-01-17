/**
 * @file       led_set.c
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
 * @brief lower 4 bits are driven to leds
 * @param[in] l integer in the range of 0 to 15
 * @retval none
 */
void
  led_set( unsigned l )
{
  l & (1<<0)?__led0_on():__led0_off();
  l & (1<<1)?__led1_on():__led1_off();
  l & (1<<2)?__led2_on():__led2_off();
  l & (1<<3)?__led3_on():__led3_off();
}