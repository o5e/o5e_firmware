/**
 * @file       err_get_ts.c
 * @headerfile err.h
 * @author     sstasiak
 * @brief      extract timestamp from err
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "fifo.h"
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
 * @brief extract timestamp from err
 * @param[in] e err
 * @retval uint64_t timestamp
 */
uint64_t
  err_get_ts( err_t const *e )
{
  trap( e );
  return e->ts;
}