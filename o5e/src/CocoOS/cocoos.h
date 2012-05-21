/*
***************************************************************************************
***************************************************************************************
***
***     File: cocoos.h
***
***     Project: cocoOS
***
***     Copyright 2009 Peter Eckstrand
***
***************************************************************************************
	This file is part of cocoOS.

    cocoOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cocoOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cocoOS.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************************


    Version: 1.0.0

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
extern uint8_t running;

uint8_t os_running( void );

#endif
