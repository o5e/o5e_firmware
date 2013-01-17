/**
 * @file       err_destroy.c
 * @headerfile err.h
 * @author     sstasiak
 * @brief      return pop'd error back to the free pool for reuse
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "bsp.h"
#include "trap.h"
#include "lifo.h"
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
 * @brief return err_t back to the free pool
 * @param[in] e previously pop'd err
 * @retval none
 * @note { threadsafe }
 */
void
  err_destroy( err_t const *e )
{
  trap( e );
  bsp_declare_state();
  bsp_disable_interrupts();
  err_free_root =
    lifo_push( err_free_root,
               e );
  bsp_enable_interrupts();
}