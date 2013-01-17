/**
 * @file       weak_ivor12_handler.c
 * @headerfile bsp_prv.h
 * @author     sstasiak
 * @brief      weak ivor12 handler
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
 * @brief default ivor12 handler, override as desired
 * @note Watchdog timer interrupt (SPR 412)
 */
#pragma push
#pragma section code_type ".handlers"
#pragma function_align 16
__declspec(interrupt)
__declspec(weak)
__declspec(section ".handlers")
void
  ivor12_handler( void )
{
  default_ivor_handler();
}
#pragma pop
/* --| PUBLIC   |--------------------------------------------------------- */