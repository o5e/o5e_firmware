/**
 * @file       led_off.c
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
 * @brief turn off led
 * @param[in] l desired led
 * @retval none
 */
void
  led_off( led_t l )
{
  switch( l )
  {
    case LED0:
      __led0_off();
      break;
    case LED1:
      __led1_off();
      break;
    case LED2:
      __led2_off();
      break;
    case LED3:
      __led3_off();
      break;
    default:
      /* TODO: push error */
  }
}