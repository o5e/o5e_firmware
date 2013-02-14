/**
 * @file       os_idle.c
 * @headerfile cocoos.h
 * @author     sstasiak
 * @brief      cocoos idle callback
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#include "cocoos.h"
#include "config.h"
#include "main.h"
#include "etpu_util.h"
#include "mpc5500_ccdcfg.h"

///#define ANGLE_TICKS_PER_DEGREE 10

/* --| TYPES    |--------------------------------------------------------- */
/* --| STATICS  |--------------------------------------------------------- */
/* --| INLINES  |--------------------------------------------------------- */
/* --| INTERNAL |--------------------------------------------------------- */

void 
  os_idle( void )
{

  /* eventually this gets gutted and we get proper updates via tpu */
  /* interrupt. for now, we keep it until I figure out a way to    */
  /* fix once and for all correctly                                */
  /*                                                               */
  /* ideally, I just put the core to sleep until the next int      */
  /* rolls in                                                      */

 // static uint32_t prev_angle = 0;
//  prev_angle = angle_clock(); // previous crank position in ticks
  //update crank shaft degree/angle clock (free running, not synced to an absolute engine position)
//  register uint32_t i;
//  register uint64_t j;

//  i = (angle_clock() - prev_angle) & 0xffffff;    // 24 bit hw counter 
//  j = i * (((uint64_t)1 << 32) / ANGLE_TICKS_PER_DEGREE);        // avoid a run time divide
//  i = (uint32_t) (j >> 32);                       // convert back to bin 0
//  if (i > 0) {                                    // delta full degrees
//    Degree_Clock += i;                          //< polled/used in Engine_OPS.c 
//    prev_angle = (prev_angle + i * ANGLE_TICKS_PER_DEGREE) & 0xffffff;
//    os_task_tick(1, (uint16_t) i);              // increment os angle clock value
//  } // if
}
/* --| PUBLIC   |--------------------------------------------------------- */

uint32_t Degree_Clock;
