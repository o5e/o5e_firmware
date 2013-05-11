/**
 * @file       bsp_timebase.c
 * @headerfile bsp.h
 * @author     sstasiak
 * @brief      timebase operations
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
 * timebase sfrs - see bug note below
 */
enum
{
  TIMEBASE_LOWER = 268,
  TIMEBASE_UPPER = 269
};

/**
 * @public
 * @brief atomic fetch of entire timebase
 * @note runs at core clock frequency
 * @retval uint64_t current time
 */
asm uint64_t
  bsp_get_timebase( void )
{

// mwerks bug ?
// the following sequence does not generate the
// correct opcodes:
//
//  mftbu   r3
//  mftb    r4
//  mftbu   r5
//
//  they show up as illegal opcodes in the disassembler
//  so I had to manually specify the sfr values:
//
//  mfspr   r3, 269 -> TIMEBASE_UPPER
//  mfspr   r4, 268 -> TIMEBASE_LOWER
//  mfspr   r5, 269 -> TIMEBASE_UPPER

  nofralloc

@loop:
  mfspr   r3, TIMEBASE_UPPER
  mfspr   r4, TIMEBASE_LOWER
  mfspr   r5, TIMEBASE_UPPER
  cmpw    r5, r3    // did upper roll ?
  bne+    @loop     // ... then lower is wrong
  blr
}

/**
 * @public
 * @brief fetch only the lower timebase
 * @note runs at core clock frequency
 * @retval uint32_t current time
 */
asm uint32_t
  bsp_get_timebase_lower( void )
{
  nofralloc
  mfspr   r3, TIMEBASE_LOWER
  blr
}
