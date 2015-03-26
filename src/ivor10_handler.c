/**
 * @file       ivor10_handler.c
 * @headerfile bsp.h
 * @author     sstasiak
 * @brief      ivor10 handler (decrementer)
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include <stdint.h>
#include "bsp.h"
#include "led.h"
#include "cocoos.h"

/*
   +------------+
   |    srr1    | +80
   +------------+
   |    srr0    | +76
   +------------+
   |     cr     | +72
   +------------+
   |     xer    | +68
   +------------+
   |     ctr    | +64
   +------------+
   |     lr     | +60
   +------------+
   |     r12    | +56
   +------------+
   |     r11    | +52
   +------------+
   |     r10    | +48
   +------------+
   |     r9     | +44
   +------------+
   |     r8     | +40
   +------------+
   |     r7     | +36
   +------------+
   |     r6     | +32
   +------------+
   |     r5     | +28
   +------------+
   |     r4     | +24
   +------------+
   |     r3     | +20
   +------------+
   |  local use | +16
   +------------+
   |  local use | +12
   +------------+
   |     r0     | +8
   +------------+
   |   Next LR  | +4
   +------------+
   | Back Chain | +0
   +------------+
*/

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */
/**
 * @brief default ivor10 handler
 * @note Decrementer interrupt (SPR 410)
 */
#pragma push
#pragma section code_type ".handlers"
#pragma function_align 16
__declspec(interrupt nowarn)
__declspec(section ".handlers")
asm void
  ivor10_handler( void )
{
  nofralloc
  stwu    rsp, -84(rsp)     /**< push entire EABI context, we're          */
  stw     r0, 8(rsp)        /*   starting a new one                       */
  mfsrr0  r0
  stw     r0, 76(rsp)       /**< push srr0                                */
  mfsrr1  r0
  stw     r0, 80(rsp)       /**< push srr1                                */
  lis     r0, (1<<11)
  mttsr   r0                /**< clr dec int                              */
  wrteei  1                 /**< unmask int's                             */
  mfcr    r0                /**< save remaining EABI context              */
  stw     r0, 72(rsp)       /**< cr                                       */
  mfxer   r0
  stw     r0, 68(rsp)       /**< xer                                      */
  mfctr   r0
  stw     r0, 64(rsp)       /**< ctr                                      */
  mflr    r0
  stw     r0, 60(rsp)       /**< lr                                       */
  stw     r12, 56(rsp)      /**< r12                                      */
  stw     r11, 52(rsp)      /**< r11                                      */
  stw     r10, 48(rsp)      /**< r10                                      */
  stw     r9, 44(rsp)       /**< r9                                       */
  stw     r8, 40(rsp)       /**< r8                                       */
  stw     r7, 36(rsp)       /**< r7                                       */
  stw     r6, 32(rsp)       /**< r6                                       */
  stw     r5, 28(rsp)       /**< r5                                       */
  stw     r4, 24(rsp)       /**< r4                                       */
  stw     r3, 20(rsp)       /**< r3                                       */
  /* use 12(rsp) and 16(rsp) for locally scoped vars as desired           */

  lis     r3, systime@h
  ori     r3, r3, systime@l
  wrteei  0
  lwz     r4, 0(r3)
  addi    r4, r4, 1         /**< systime++ ATOMICALLY, ignore roll        */
  stw     r4, 0(r3)
  wrteei  1

/*  li      r3, LED0         temp remoe, mcb
  bl      led_invert        /**< toggle led for fun                       */

  xor     r3, r3, r3        /**< 'master' clock tick:                     */
  li      r4, 1             /*   os_task_tick( 0, 1 );                    */
  bl      os_task_tick

  lwz     r3, 20(rsp)       /**< r3                                       */
  lwz     r4, 24(rsp)       /**< r4                                       */
  lwz     r5, 28(rsp)       /**< r5                                       */
  lwz     r6, 32(rsp)       /**< r6                                       */
  lwz     r7, 36(rsp)       /**< r7                                       */
  lwz     r8, 40(rsp)       /**< r8                                       */
  lwz     r9, 44(rsp)       /**< r9                                       */
  lwz     r10, 48(rsp)      /**< r10                                      */
  lwz     r11, 52(rsp)      /**< r11                                      */
  lwz     r12, 56(rsp)      /**< r12                                      */
  lwz     r0, 60(rsp)
  mtlr    r0                /**< lr                                       */
  lwz     r0, 64(rsp)
  mtctr   r0                /**< ctr                                      */
  lwz     r0, 68(rsp)
  mtxer   r0                /**< xer                                      */
  lwz     r0, 72(rsp)
  mtcr    r0                /**< cr                                       */
  wrteei  0
  lwz     r0, 80(rsp)
  mtsrr1  r0                /**< pop srr1                                 */
  lwz     r0, 76(rsp)
  mtsrr0  r0                /**< pop srr0                                 */
  lwz     r0, 8(rsp)        /**< pop r0                                   */
  addi    rsp, rsp, 84
  rfi
}
#pragma pop
/* --| PUBLIC   |--------------------------------------------------------- */