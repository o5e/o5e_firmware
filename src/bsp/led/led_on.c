/**
 * @file       led_on.c
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
 * @brief turn on led
 * @param[in] l desired led
 * @retval none
 */
void
  led_on( led_t l )
{
  switch( l )
  {
    case LED0:
      __led0_on();
      break;
    case LED1:
      __led1_on();
      break;
    case LED2:
      __led2_on();
      break;
    case LED3:
      __led3_on();
      break;
    default:
      /* TODO: push error */
  }
}