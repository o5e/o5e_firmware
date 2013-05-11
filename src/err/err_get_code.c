/**
 * @file       err_get_code.c
 * @headerfile err.h
 * @author     sstasiak
 * @brief      extract error code from err
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
 * @brief extract err code from err
 * @param[in] e err
 * @retval uint32_t code
 */
uint32_t
  err_get_code( err_t const *e )
{
  trap( e );
  trap( e->code != CODE_NONE );
  return e->code;
}