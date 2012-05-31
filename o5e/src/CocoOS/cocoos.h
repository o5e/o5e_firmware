/*
 * Copyright (c) 2012 Peter Eckstrand
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the cocoOS operating system.
 * Author: Peter Eckstrand <info@cocoos.net>
 */
 
/****************************************************************************************


    Change log:
    2009-07-06: 1.0.0 First release
    2010-07-21: New definition of OS_END that kills the running task. A task reaching the
    OS_END, will be killed. To make a task execute forever, a for(;;) loop must be implemented
    before the OS_END macro.

    2010-11-01: Replaced the return 0; with return; in the macros, so the task procedures
                can have return type void.

    2010-11-16: Change to uint16_t for the state variable in OS_BEGIN macro. Renamed the 
                state variable.
				
	2011-12-08: Changed the OS_WAIT_TICKS macro to include a clock id
    
    2012-01-04: Released under BSD license.

***************************************************************************************
*/
#ifndef COCOOS_H
#define COCOOS_H

/** @file cocoos.h cocoOS kernel header file*/

#include <inttypes.h>
#include "os_port.h"
#include "os_typedef.h"
#include "os_defines.h"
#include "os_event.h"
#include "os_sem.h"
#include "os_task.h"
#include "os_assert.h"
#include "os_msgqueue.h"
#include "os_applAPI.h"



#define NO_TID	        255
#define NO_EVENT        255
#define NO_QUEUE        255
#define NO_SEM          255

/* Total number of semaphores needed */
#define N_TOTAL_SEMAPHORES    ( N_SEMAPHORES + N_QUEUES )


/* Total number of events needed */
#define N_TOTAL_EVENTS        ( N_EVENTS + N_QUEUES )








#define OS_BEGIN            uint16_t os_task_state = os_task_internal_state_get(running_tid); switch ( os_task_state ) { case 0:

#define OS_END	            os_task_kill(running_tid);\
                            running_tid = NO_TID;\
					        return;}

#define OS_SCHEDULE         os_task_internal_state_set(running_tid, __LINE__);\
                            running_tid = NO_TID;\
					        return;\
					        case __LINE__:




#define OS_WAIT_TICKS(x,y)	do {\
								os_task_wait_time_set( running_tid, y, x );\
								OS_SCHEDULE;\
						   	   } while ( 0 )



extern uint8_t running_tid;
extern uint8_t last_running_task;
extern uint8_t running;

uint8_t os_running( void );

#endif
