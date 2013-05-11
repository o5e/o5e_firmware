/**
 * @file       err_pop.c
 * @headerfile err.h
 * @author     sstasiak
 * @brief      pop oldest error from the stack
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "led.h"
#include "bsp.h"
#include "fifo.h"
#include "err.h"
#include "err_prv.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public
 * @brief pop oldest error from the stack
 * @param none
 * @retval err_t* err or 0 if none
 * @note { threadsafe }
 */
err_t const *
  err_pop( void )
{
  bsp_declare_state();
  bsp_disable_interrupts();
  err_t const * const e =
    fifo_pop( &err_fifo_root );
  if( !e )
    led_off( ERR_LED );           /**< clear led when stack is pop'd clean */
  bsp_enable_interrupts();
  return e;
}