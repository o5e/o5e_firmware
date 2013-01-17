/**
 * @file       err_init.c
 * @headerfile err.h
 * @author     sean
 * @brief      init code logger
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "lifo.h"
#include "fifo.h"
#include "trap.h"
#include "led.h"
#include "err.h"
#include "err_prv.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
err_t err_pool[ERR_DEPTH];
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
err_t *err_free_root = 0;
fifo_t err_fifo_root;
/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public
 * @brief init code logger
 * @retval none
 * @note { not threadsafe }
 */
void
  err_init( void )
{
  trap( ERR_DEPTH );
  fifo_init( &err_fifo_root );
  for(register i=0; i < ERR_DEPTH ;i++)    /**< loadup free pool          */
  {
    err_free_root =
      lifo_push( err_free_root,
                 &err_pool[i] );
    trap( err_free_root );
  }
  led_off( ERR_LED );
}