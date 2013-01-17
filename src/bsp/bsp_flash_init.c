/**
 * @file       bsp_flash_init.c
 * @headerfile
 * @author     sstasiak
 * @brief      init 5634 flash on startup
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
void bsp_flash_init( void );

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/**
 * @internal
 * @brief perform word write from ram
 * void __set_value( uint32_t value, uint32_t addr, uint32_t off )
 */
uint32_t const __set_value[] =
{
  0x7c64292e,   /**< stwx r3, r4, r5 */
  0x7c0004ac,   /**< msync           */
  0x4c00012c,   /**< isync           */
  0x4e800020    /**< blr             */
};

/* --| PUBLIC   |--------------------------------------------------------- */
/**
 * @public but only called on startup
 * @brief reduce flash waitstates to match 80MHz operation for OEM hardware
 *        on a 5634 - nothing more
 * @param none
 * @retval none
 * @attention NO STACK AVAILABLE for your use here
 */
__declspec(section ".init")
asm void
  bsp_flash_init( void )
{

extern uint32_t _internal_ram_end;
enum { BIUCR = 0xC3F8801Cul };

    nofralloc
    mflr  r7                  /**< no stack available here      */

    lis   r5, __set_value@h   /**< __set_value fcall - source   */
    ori   r5, r5, __set_value@l
                              /**< __set_value fcall - dest     */
    lis   r6, _internal_ram_end@h
    ori   r6, r6, (_internal_ram_end-sizeof(__set_value))@l
                              /**< unrolled copy                */
    lwz   r4, 0(r5)           /**< __set_value[0]               */
    stw   r4, 0(r6)
    lwz   r4, 4(r5)           /**< __set_value[1]               */
    stw   r4, 4(r6)
    lwz   r4, 8(r5)           /**< __set_value[2]               */
    stw   r4, 8(r6)
    lwz   r4, 12(r5)          /**< __set_value[3]               */
    stw   r4, 12(r6)

    lis   r4, BIUCR@h         /**< BIUCR (base)                 */
    ori   r4, r4, BIUCR@l     /*                                */

    lis   r3, 0x0001          /**< 82MHz operation, I+D prefetch*/
    ori   r3, r3, 0x6B55      /*   enabled                      */
    xor   r5, r5, r5          /**< BIUCR+0=BIUCR                */
    mtlr  r6                  /**< addr of __set_value()        */
    blrl

    lis   r3, (3<<14)         /**< 3 buf inst, 1 buf data       */
    li    r5, 8               /**< BIUCR+8=BIUCR2               */
    mtlr  r6                  /**< addr of __set_value()        */
    blrl

    xor   r3, r3, r3          /**< NO prefetching on Bank1      */
    li    r5, 12              /**< BIUCR+12=PFCR3               */
    mtlr  r6                  /**< addr of __set_value()        */
    blrl

    mtlr  r7
    blr
}