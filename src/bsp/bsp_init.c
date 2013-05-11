/**
 * @file       bsp_init.c
 * @headerfile
 * @author     sstasiak
 * @brief      final board setup/config before dropping into main()
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "mpc563xm.h"
#include "bsp.h"

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */

__declspec(weak)
void
  bsp_decr_init( void )
{
}

/* --| PUBLIC   |--------------------------------------------------------- */

/**
 * @brief system time in milliseconds since power on.
 * @warning { READ-ONLY }
 */
uint32_t systime = 0;

/**
 * @public
 * @brief Final initialization before dropping into mainline code
 * @details Safe for c/c++ since the runtime is already up and any static
 *          classes are already initialized. Typically, this is the final
 *          step before dropping into main()
 */
void
  bsp_init( void )
{
  /* install ivor handlers                                              */
  extern unsigned long _f_handlers;         /**< linker defined         */
  asm
  {
    lis      r3, _f_handlers@h
    ori      r3, r3, _f_handlers@l
    mtivpr   r3

    li       r3, ivor0_handler@l
    mtivor0  r3   /**< Critical input (SPR 400)                         */
    li       r3, ivor1_handler@l
    mtivor1  r3   /**< Machine check interrupt (SPR 401)                */
    li       r3, ivor2_handler@l
    mtivor2  r3   /**< Data storage interrupt (SPR 402)                 */
    li       r3, ivor3_handler@l
    mtivor3  r3   /**< Instruction storage interrupt (SPR 403)          */
    li       r3, ivor4_handler@l
    mtivor4  r3   /**< External input interrupt (SPR 404)               */
    li       r3, ivor5_handler@l
    mtivor5  r3   /**< Alignment interrupt (SPR 405)                    */
    li       r3, ivor6_handler@l
    mtivor6  r3   /**< Program interrupt (SPR 406)                      */
    li       r3, ivor7_handler@l
    mtivor7  r3   /**< Floating-point unavailable interrupt (SPR 407)   */
    li       r3, ivor8_handler@l
    mtivor8  r3   /**< System call interrupt (SPR 408)                  */
    li       r3, ivor9_handler@l
    mtivor9  r3   /**< Auxiliary processor (SPR 409)                    */
    li       r3, ivor10_handler@l
    mtivor10 r3   /**< Decrementer interrupt (SPR 410)                  */
    li       r3, ivor11_handler@l
    mtivor11 r3   /**< Fixed-interval timer interrupt (SPR 411)         */
    li       r3, ivor12_handler@l
    mtivor12 r3   /**< Watchdog timer interrupt (SPR 412)               */
    li       r3, ivor13_handler@l
    mtivor13 r3   /**< Data TLB error interrupt (SPR 413)               */
    li       r3, ivor14_handler@l
    mtivor14 r3   /**< Instruction TLB error interrupt (SPR 414)        */
    li       r3, ivor15_handler@l
    mtivor15 r3   /**< Debug interrupt (SPR 415)                        */
    li       r3, ivor32_handler@l
    mtivor32 r3   /**< SPE APU unavailable interrupt (SPR 528)          */
    li       r3, ivor33_handler@l
    mtivor33 r3   /**< SPE floating-point data exception int (SPR 529)  */
    li       r3, ivor34_handler@l
    mtivor34 r3   /**< SPE floating-point round exception int (SPR 530) */
  }
  /* config INTC                                                        */
  INTC.MCR.R = 0;                       /**< s/w mode, 4byte entry size */
  extern unsigned long vectors[];       /**< in vectors.c               */
  INTC.IACKR.R = (unsigned long)vectors;/**< set base                   */
  INTC.CPR.B.PRI = 0;                   /**< unmask                     */

  bsp_decr_init();            /**< if you override this, make sure      */
                              /*   you've already overridden ivor10 too */
}