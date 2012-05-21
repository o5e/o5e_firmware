/*
***************************************************************************************
***************************************************************************************
***
***     File: os_task.h
***
***     Project: cocoOS
***
***     Copyright 2009, 2010 Peter Eckstrand
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


    Version: 1.2.0

    Change log:
    2009-07-06: 1.0.0: First release
	2010-03-02: 1.1.0: Added OS_SUSPEND_TASK()
	2010-04-13: 1.2.0: Modified OS_SUSPEND_TASK() to take task id as parameter. Any task
					   can suspend any other task.
					   Added OS_RESUME_TASK() that restores the state of the task to the 
					   state it had when it was suspended
    2010-07-14: Modified the Suspend and Resume macros to take task function pointers
                as input arguments.
                Moved and renamed os_task_create and os_task_state_get to os_applAPI.h

    2010-07-21: Added function for killing a task.

    2010-11-17: Return type uint16_t for os_task_internal_state_get()
	2011-12-08: os_task_wait_time_set takes an id parameter to specify which clock to wait for
	2011-12-14: os_task_tick takes a tick size parameter

***************************************************************************************
*/
#ifndef OS_TASK_H__
#define OS_TASK_H__

/** @file os_task.h Task header file*/

#include "os_defines.h"
#include "os_msgqueue.h"

typedef struct tcb tcb;

typedef enum {
    SUSPENDED,
    WAITING_SEM,
    WAITING_TIME,
    WAITING_EVENT,
    READY,
    RUNNING,
    KILLED
} TaskState_t;




#define OS_SUSPEND_TASK( proc )    do {\
								        os_task_suspend( task_id_get( proc ) );\
                                        if ( task_id_get( proc ) == running_tid ) {\
                                            OS_SCHEDULE;\
                                        }\
							  	    } while (0)




#define OS_RESUME_TASK( proc )		do {\
								    os_task_resume( task_id_get( proc ) );\
                                    if ( task_id_get( proc ) == running_tid ) {\
                                        OS_SCHEDULE;\
                                    }\
							  	} while (0)



uint8_t os_task_highest_prio_ready_task( void );
void os_task_ready_set( uint8_t tid );
void os_task_wait_sem_set( uint8_t tid, Sem_t sem );
void os_task_suspend( uint8_t tid );
void os_task_resume( uint8_t tid );
void os_task_kill( uint8_t tid );
uint8_t os_task_prio_get( uint8_t tid );
void os_task_clear_wait_queue( uint8_t tid );
void os_task_wait_time_set( uint8_t tid, uint8_t id, uint16_t time );
void os_task_wait_event( uint8_t tid, Evt_t eventId, uint8_t waitSingleEvent );
void os_task_tick( uint8_t id, uint16_t tickSize );
void os_task_signal_event( Evt_t eventId );
void os_task_run_highest( void );
uint16_t os_task_internal_state_get( uint8_t tid );
void os_task_internal_state_set( uint8_t tid, uint16_t state );
void os_task_release_highest_prio_task( Sem_t sem );
uint8_t os_task_waiting_this_semaphore( Sem_t sem );
MsgQ_t os_task_msgQ_get( uint8_t tid );





#endif
