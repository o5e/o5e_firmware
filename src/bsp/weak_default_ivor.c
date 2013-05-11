/**
 * @file       weak_default_ivor.c
 * @headerfile bsp_prv.h
 * @author     sstasiak
 * @brief      default ivor handler, unless the application overrides it
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "bsp.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/**
 * @brief default ivor handler, override as desired
 */
#pragma push
#pragma section code_type ".handlers"
#pragma function_align 16
__declspec(interrupt)
__declspec(weak)
__declspec(section ".handlers")
void
default_ivor_handler( void )
{
  while( 1 )
  {
  }
}
#pragma pop
/* --| PUBLIC   |--------------------------------------------------------- */