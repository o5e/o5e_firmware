/**
 * @file       bsp_decr_init.c
 * @headerfile bsp.h
 * @author     sstasiak
 * @brief      initialize decrementer to overflow at 1000Hz
 *             for an 80Mhz core clock, ivor10 handles the other
 *             half of this
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
/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public
 * @brief decrementer init hook, override as desired
 */
asm void
  bsp_decr_init( void )
{
  enum { RATE = ((80000000/1000)-1) }; /**< 1000Hz at 80MHz                */
  fralloc

  xor     r0, r0, r0
  mttbu   r0
  mttbl   r0                           /**< wipe timebase                  */
  lis     r0, RATE@h
  ori     r0, r0, RATE@l
  mtdecar r0
  mtdec   r0                           /**< reload = initial               */
  mftcr   r0
  lis     r3, 0x0440                   /**< TCR[DIE] | TCR[ARE]            */
  or      r0, r0, r3
  mttcr   r0

  frfree
  blr
}