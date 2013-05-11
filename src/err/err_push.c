/**
 * @file       err_push.c
 * @headerfile err.h
 * @author     sean
 * @brief      push an error code into the stack
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "led.h"
#include "lifo.h"
#include "fifo.h"
#include "bsp.h"
#include "trap.h"
#include "err.h"
#include "err_prv.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public
 * @brief push an error code into the stack
 * @param[in] code error code
 * @retval none
 * @note { threadsafe }
 */
void
  err_push( uint32_t code )
{
  bsp_declare_state();
  trap( code != CODE_NONE );       /**< why are you pushing CODE_NONE ? */

  /* the lifo/fifo api is not threadsafe, and I want any context to feel free
   * to call this api. wrapping in a critical section ensures threadsafety.
   */

  bsp_disable_interrupts();
  err_t *const e =
    lifo_pop( &err_free_root );
  bsp_enable_interrupts();

  if( e )
  {
    e->ts = bsp_get_timebase();       /**< store off info               */
    e->code = code;

    bsp_disable_interrupts();
    fifo_push( &err_fifo_root,
               e );
    led_on( ERR_LED );
    bsp_enable_interrupts();
  }
  else
  {
    /* no more blocks - signalling an error won't help much either */
  }
}