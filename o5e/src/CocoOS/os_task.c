/*
***************************************************************************************
***************************************************************************************
***
***     File: os_task.c
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


    Version: 1.1.0

    Change log:
    2009-07-06: 1.0.0: First release
	2010-04-13: 1.1.0: os_task_create() returns the task id of the created task. 
	                   A variable 'savedState' was added to the tcb structure. This is
					   used for saving the current task state when the task is suspended.
					   When the task is resumed again, the state is restored.
					   os_task_suspend( uint8_t tid ), and os_task_resume( uint8_t tid )
					   was added. Modified os_task_signal_event( uint8_t evId ) so that 
					   a task that is in pending state does not get its eventQueue flags
					   cleared if an event is signaled when the task is suspended.

    2010-06-13: Removed function for retrieving a pointer to the task procedure. Added
                os_task_run_highest(), that runs the highest priority task.

    2010-06-19: Added os_task_tid_get() that finds the task id associated with a specified
                task procedure. Can be used with the SUSPEND/RESUME macros. 
                Added os_task_state_get().

    2010-07-01: Added a number of asserts in os_task_create() in order to prevent creating
                invalid tasks.

    2010-07-20: Added a semaphore pointer to the tcb structure. This pointer is set when
                waiting for a semaphore and is used when the task is suspended to remove the 
                task from that semaphores waiting list. 
                When a task waiting for a semaphore is suspended its internal state is reset
                and the task state is set to READY, so that when resumed, the task will start
                over instead of wait for the semaphore. 

    2010-07-21: Added functions for killing a task.

    2010-07-26: Added functions for the new semaphore handling.

    2010-10-06: Changed the task list to consist of tcb's instead of tcb pointers. Removed
                use of malloc.
                The N_TASKS has to be defined in os_defines.h and shall match the number
                of used tasks in the application.
    
    2010-10-14: Added constraint when suspending a task. If a task is KILLED it can not
                be suspended and resumed.

    2010-10-20: Added Message queue as tcb member. When creating a task, a message queue is
                set up for the task.
                
    2010-10-23: If the message pool size is set to 0 when creating a task, no message
                queue is used for that task. The messageQueue member will be set to
                NO_QUEUE.
                
    2010-11-01: Changed the assert in os_task_run_highest() to check against nTasks 
                instead of MAX_TASKS.

    2010-11-17: Bugfix: Return type uint16_t for os_task_internal_state_get().

    2011-01-03: The tick function calls the message queue tick function to update the 
                message delay counters.
				
	2011-12-08: Support for sub clocks. Added clockId to the tcb struct. A clock id is specified when setting the 
				task wait time. os_task_tick takes an id parameter.
				
	2011-12-14: os_task_tick takes a tick size parameter

***************************************************************************************
*/


#include "cocoos.h"

#include <stdlib.h>


struct tcb {
    uint8_t tid;
    uint8_t prio;
    TaskState_t state;
    TaskState_t savedState;
    Sem_t semaphore;
    uint16_t internal_state;
    EventQueue_t eventQueue;
    MsgQ_t msgQ;
    uint8_t waitSingleEvent;
    uint16_t time;
	uint8_t clockId;
    taskproctype taskproc;
};


static void task_wait_sem_set( uint8_t tid, Sem_t sem );
static void task_suspended_set( uint8_t tid );
static void task_waiting_time_set( uint8_t tid );
static void task_waiting_event_set( tcb *task );
static void task_ready_set( uint8_t tid );
static void task_killed_set( uint8_t tid );

static tcb task_list[ N_TASKS ];
static uint8_t nTasks = 0;


/************************************************************** *******************/
/*  uint8_t task_create( taskproctype taskproc, uint8_t prio, Msg_t *msgPool, uint8_t poolSize, uint16_t msgSize )    *//**
*   
*   Creates a task scheduled by the os. The task is put in the ready state.
*
*		@param taskproc Function pointer to the task procedure.
*		@param prio Task priority on a scale 0-255 where 0 is the highest priority.
*       @param msgPool Pointer to the message pool, containing messages. Ignored if poolSize is 0.
*       @param poolSize Size of the message pool. Set to 0 if no message pool needed for the task
*       @param msgSize Size of the message type held in the message queue
*		@return Task id of the created task.
*
*		@remarks \b Usage: @n Should be called early in system setup, before starting the task 
*       execution. Only one task per priority level is allowed.
*	
*		
*       @code
static uint8_t taskId;
static Msg_t msgpool_1[ POOL_SIZE ];

int main(void) {
	system_init();
	os_init();
	taskId = task_create( myTaskProc, 1, msgpool_1, POOL_SIZE, sizeof(Msg_t) );
	...
}
*		@endcode
*       
*/
/*********************************************************************************/
uint8_t task_create( taskproctype taskproc, uint8_t prio, Msg_t *msgPool, uint8_t poolSize, uint16_t msgSize ) {
    uint8_t taskId;
    tcb *task;

    os_assert( os_running() == 0 );
    os_assert( nTasks < N_TASKS );
    os_assert( taskproc != NULL );

    taskId = nTasks;

    /* Check that no other task has the same prio */
    while ( taskId != 0 ) {
        --taskId;
        os_assert( task_list[ taskId ].prio != prio );
    } 
    
    
    task = &task_list[ nTasks ];

    task->tid = nTasks;
    task->prio = prio;
    task->state = READY;
    task->savedState = READY;
    task->semaphore = 0;
    task->internal_state = 0;
    task->taskproc = taskproc;
    task->waitSingleEvent = 0;
    task->time = 0;
    if ( poolSize > 0 ) {
        task->msgQ = os_msgQ_create( msgPool, poolSize, msgSize );
    }
    else {
        task->msgQ = NO_QUEUE;
    }

    os_task_clear_wait_queue( nTasks );
    
    nTasks++;
    return task->tid;
}


/*********************************************************************************/
/*  TaskState_t task_state_get( taskproctype taskproc )                                              *//**
*   
*   Gets the current state of the task associated with the specified procedure
*
*		@param taskproc Function pointer to the task procedure.
*
*		@return State of the task
*	
*		
*       @code
static void led_task500(void)
{
    task_open();	
    
    for (;;) {
        LED_TOGGLE(0);
        
        task_wait( 500 );
    }

    task_close();
}


static void led_task1000(void)
{
    task_open();	

    for (;;) {
        sem_wait( buttonSem );

        if ( buttonPressed ) {
            if ( SUSPENDED < task_state_get( led_task500 ) ) {
                task_suspend( led_task500 );
            }

        }
        else {
            if ( SUSPENDED == task_state_get( led_task500 ) ) {
                task_resume( led_task500 );
            }
        }

        sem_signal( buttonSem );

        LED_TOGGLE(1);


        task_wait( 900 );
    }

    task_close();

}
*		@endcode
*       
*/
/*********************************************************************************/
TaskState_t task_state_get( taskproctype taskproc ) {
    uint8_t tid = task_id_get( taskproc );
    os_assert( tid != NO_TID );
    return task_list[ tid ].state;
}


/*********************************************************************************/
/*  uint8_t task_id_get( taskproctype taskproc )                                             *//**
*   
*   Gets the task id of the task associated with the specified task procedure
*
*		@param taskproc Function pointer to the task procedure.
*
*		@return TaskId of the task
*	
*		
*       @code
static void waitingTask(void)
{
    task_open()

    event_wait( event );

    if ( event_signaling_taskId_get( event ) == task_id_get( signalingTask1 ) ) {
        ...
    }

	task_close();

}


static void signalingTask1(void)
{
	task_open();	
	
	task_wait( 900 );
       
    event_signal(event);

	task_close();

}
*		@endcode
*       
*/
/*********************************************************************************/
uint8_t task_id_get( taskproctype taskproc ) {
    uint8_t index;

    for ( index = 0; index != nTasks; index++ ) {
        if ( task_list[ index ].taskproc == taskproc ) {
            return index;
        }
    }
    return NO_TID;
}


/*********************************************************************************/
/*  void task_kill( taskproctype taskproc )                                   *//**
*   
*   Puts the task associated with the specified task procedure in the killed state. 
*   A killed task, cannot be resumed.
*
*   @param taskproc Function pointer to the task procedure.
*
*	@return None
*	
*		
*   @code
static void waitingTask(void)
{
    task_open();

    event_wait( event );

    if ( event_signaling_taskId_get( event ) == task_id_get( signalingTask1 ) ) {
        task_kill( signalingTask1 );
    }

	task_close();

}


static void signalingTask1(void)
{
	task_open();	
	
	task_wait( 900 );
       
    event_signal(event);

	task_close();

}
*		@endcode
*       
*/
/*********************************************************************************/
void task_kill( taskproctype taskproc ) {
    os_task_kill( task_id_get( taskproc ) );
}


/* Finds the task with highest prio that are ready to run */
uint8_t os_task_highest_prio_ready_task( void ) {
    uint16_t index;
    tcb *task;
    uint8_t highest_prio_task = NO_TID;
    uint8_t highest_prio = 255;
    TaskState_t state;
    uint8_t prio;
    
    os_disable_interrupts();
    
    for ( index = 0; index != nTasks; ++index ) {
        task = &task_list[ index ];
        prio = task->prio;
        state = task->state;

        if ( READY == state ) {
            if ( prio < highest_prio ) {
                highest_prio = prio;
                highest_prio_task = index;
            }
        }
    }

	os_enable_interrupts();
    return highest_prio_task;
}


/* Finds the task with highest prio waiting for sem, and makes it ready to run */
void os_task_release_highest_prio_task( Sem_t sem ) {
    uint8_t highestPrio = 255;
    uint8_t tid;
    uint8_t foundTask = NO_TID;
    uint8_t taskIsWaitingForThisSemaphore;
    tcb *task;

    for ( tid = 0; tid != nTasks; ++tid ) {
        task = &task_list[ tid ];
        taskIsWaitingForThisSemaphore = (( task->state == WAITING_SEM ) && ( task->semaphore == sem ) );

        if ( taskIsWaitingForThisSemaphore == 1 ) {
            if ( task->prio < highestPrio ) {
                highestPrio = task->prio;
                foundTask = tid;
            }
        }
    }

    /* We have found a waiting task. */
    if ( NO_TID != foundTask ) {
        task_list[ foundTask ].state = READY;
    }
}


/* Checks if any task is waiting for this semaphore */
uint8_t os_task_waiting_this_semaphore( Sem_t sem ) {
    uint8_t tid;
    tcb *task;
    uint8_t taskIsWaitingForThisSemaphore;
    uint8_t result = 0;

    for ( tid = 0; tid != nTasks; ++tid ) {
        task = &task_list[ tid ];
        taskIsWaitingForThisSemaphore = (( task->state == WAITING_SEM ) && ( task->semaphore == sem ) );

        if ( taskIsWaitingForThisSemaphore == 1 ) {
            result = 1;
            break;
        }
    }

    return result;
}


/* Sets the task to wait for semaphore state */
void os_task_wait_sem_set( uint8_t tid, Sem_t sem ) {
    os_assert( tid < nTasks );
    task_wait_sem_set( tid, sem );
}


/* Sets the task to ready state */
void os_task_ready_set( uint8_t tid ) {
    os_assert( tid < nTasks );
    task_ready_set( tid );
}


void os_task_suspend( uint8_t tid ) {
    TaskState_t state;

    os_assert( tid < nTasks );

    state = task_list[ tid ].state;
    
    if ( state != KILLED ) {
        /* If a task is waiting for a semaphore when beeing suspended, there is a risk      */
        /* that the semaphore will be signaled while the task is suspended, and if the task */
        /* is then resumed it could hang if the semaphore is not signaled again. Therefore  */
        /* the task is reset when it is resumed. */
        if ( WAITING_SEM == state ) {
            task_list[ tid ].savedState = READY;
            task_list[ tid ].internal_state = 0;
        }
        else {
            task_list[ tid ].savedState = state;
        }

        task_suspended_set( tid );
    }
}


void os_task_resume( uint8_t tid ) {
    
    os_assert( tid < nTasks );

    if ( task_list[ tid ].state == SUSPENDED ) {
	    task_list[ tid ].state = task_list[ tid ].savedState;
    }
}


void os_task_kill( uint8_t tid ) {
    os_assert( tid < nTasks );
    task_killed_set( tid );

}


uint8_t os_task_prio_get( uint8_t tid ) {
    os_assert( tid < nTasks );
    return task_list[ tid ].prio;
    
}


/* Clears the event wait queue of a task */
void os_task_clear_wait_queue( uint8_t tid ) {
    uint8_t event;
    tcb *task;

    task = &task_list[ tid ];

    task->waitSingleEvent = 0;
    event = EVENT_QUEUE_SIZE;

    do {
        --event;
        task->eventQueue.eventList[ event ] = 0;
    } while ( event != 0 );
    
}


/* Checks if a tasks event wait queue is empty or not */
uint8_t os_task_wait_queue_empty( uint8_t tid ) {
    uint8_t event;
    uint8_t result;

    result = 1;
    event = EVENT_QUEUE_SIZE;

    do {
        --event;
        if ( task_list[ tid ].eventQueue.eventList[ event ] != 0 ) {
            result = 0;
        }
    } while ( event != 0 );
        

    return result;
}


void os_task_wait_time_set( uint8_t tid, uint8_t id, uint16_t time ) {
    os_assert( tid < nTasks );
    os_assert( time > 0 );
    
    task_list[ tid ].clockId = id;
    task_list[ tid ].time = time;
    task_waiting_time_set( tid );
}


void os_task_wait_event( uint8_t tid, Evt_t eventId, uint8_t waitSingleEvent ) {
    uint8_t eventListIndex;
    uint8_t shift;
    tcb *task;
    
    os_assert( tid < nTasks );

    task = &task_list[ tid ];

    eventListIndex = eventId / 8;
    shift = eventId & 0x07;

    task->eventQueue.eventList[ eventListIndex ] |= 1 << shift;
    task->waitSingleEvent = waitSingleEvent;
    task_waiting_event_set( task );
}


void os_task_tick( uint8_t id, uint16_t tickSize ) {
    uint8_t index;
    
    /* Search all tasks and decrement time for waiting tasks */
    for ( index = 0; index != nTasks; ++index ) {
        if ( task_list[ index ].state == WAITING_TIME ) {
            
            /* Found a waiting task, is it ready? */
            if ( task_list[ index ].clockId == id ) {
				if ( task_list[ index ].time <= tickSize ) {
					task_list[ index ].time = 0;
					task_ready_set( index );
				}
				else {
					task_list[ index ].time -= tickSize;
				}					
            }
        }

        /* If the task has a message queue, decrement the delayed message timers */
        if ( id == 0 ) {
            if ( task_list[ index ].msgQ != NO_QUEUE ) {
                os_msgQ_tick( task_list[ index ].msgQ );
            }
        }
        
    }
}


void os_task_signal_event( Evt_t eventId ) {
    uint8_t index;
    uint8_t taskWaitingForEvent;
    uint8_t eventListIndex;
    uint8_t shift;

    eventListIndex = eventId / 8;
    shift = eventId & 0x07;

    for ( index = 0; index != nTasks; index++ ) {

        taskWaitingForEvent = task_list[ index ].eventQueue.eventList[eventListIndex] & (1<<shift);
        
        if (( taskWaitingForEvent ) && ( task_list[ index ].state == WAITING_EVENT )) {
            
            task_list[ index ].eventQueue.eventList[eventListIndex] &= ~(1<<shift);
            
            if ( task_list[ index ].waitSingleEvent || os_task_wait_queue_empty( index ) ) {
                os_task_clear_wait_queue( index );
                task_ready_set( index );
            }
        }
    }
}


/* Run the highest priority task ready for execution. Assumes running_tid has been assigned */
/* the task id of the highest priority ready task */
void os_task_run_highest( void ) {
    os_assert( running_tid < nTasks );
    task_list[ running_tid ].taskproc();
}


uint16_t os_task_internal_state_get( uint8_t tid ) {
    return task_list[ tid ].internal_state;
}


void os_task_internal_state_set( uint8_t tid, uint16_t state ) {
    task_list[ tid ].internal_state = state;
}


MsgQ_t os_task_msgQ_get( uint8_t tid ) {
    return task_list[ tid ].msgQ;
}


static void task_wait_sem_set( uint8_t tid, Sem_t sem ) {
    task_list[ tid ].state = WAITING_SEM;
    task_list[ tid ].semaphore = sem;
}


static void task_ready_set( uint8_t tid ) {
    task_list[ tid ].state = READY;
}


static void task_suspended_set( uint8_t tid ) {
    task_list[ tid ].state = SUSPENDED;
}


static void task_waiting_time_set( uint8_t tid ) {
    task_list[ tid ].state = WAITING_TIME;
}


static void task_waiting_event_set( tcb *task ) {
    task->state = WAITING_EVENT;
}


static void task_killed_set( uint8_t tid ) {
    task_list[ tid ].state = KILLED;
}
