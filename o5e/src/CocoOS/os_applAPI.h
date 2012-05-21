/*
***************************************************************************************
***************************************************************************************
***
***     File: os_applAPI.h
***
***     Project: cocoOS
***
***     Copyright 2010 Peter Eckstrand
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
    2010-07-15: 1.0.0: First release
    2010-11-16: New macro for posting messages which takes care of releasing the semaphore
                and waiting for the event in case of full buffer.

    2011-01-03: New macro for posting a message with a delay.
	2011-12-04: Added the sleep callback function.
	2011-12-08: Support for sub clocks. Added macro task_wait_id(id,n) that puts the task to
				wait for n ticks of the id sub clock.
	2011-12-14: Added os_sub_nTick, for incrementing sub clocks in steps > 1
	

***************************************************************************************
*/
#ifndef _os_applapi_h__
#define _os_applapi_h__

#include "cocoos.h"


/** @file os_applAPI.h cocoOS API header file*/
/*********************************************************************************/
/*  task_suspend( taskproc )                                                 *//**
*   
*   Macro for suspending a task
*
*		@param taskproc Task procedure of the task to suspend
*
*		@remarks If the task to suspend is the same as the current running task, the 
task is immediately suspended and other tasks are scheduled to execute. 
If the task to suspend is another than the current task, that task is immediately put 
in suspended state, and the current task continues to execute. @n \b Usage: @n 
* @code 

static Msg_t msgPool[ POOL_SIZE ];

static void led_task(void) {
 task_open();	
  for(;;) {
	led_toggle();
	task_wait( 100 );
  }
 task_close();
}


static void button_task(void) {
 task_open();
 for (;;) {
    if ( buttonPressed ) {
        task_suspend( led_task );
    }
    task_wait(10);
 }
 task_close();
}


int main( void ) {
    ...
    task_create( led_task, 1, msgPool, POOL_SIZE, sizeof(Msg_t) );
    task_create( button_task, 2, NULL, 0, 0 );
	...
}

 @endcode 
 *******************************************************************************/
#define task_suspend( taskproc )    OS_SUSPEND_TASK( taskproc )


 /*********************************************************************************/
/*  task_resume(taskproc)                                                 *//**
*   
*   Macro for resuming a task
*
*		@param taskproc Task procedure of the task to resume
*
*		@remarks The macro has no effect if the task is not in the SUSPENDED state @n
*       If the task was waiting for a semaphore when it was suspended, the task will be reset and @n
* execution will restart from the task procedure top.@n \b Usage: @n 
* @code 


static void led_task(void) {
 task_open();	
  for(;;) {
	led_toggle();
	task_wait( 100 );
  }
 task_close();
}


static void button_task(void) {
 task_open();
 for (;;) {
     if ( button_1_Pressed ) {
       task_suspend( led_task );
     }
     else if ( button_2_Pressed ){
       task_resume( led_task );
     }
 }
 task_close();
}


int main( void ) {
	...
	task_create( led_task, 1, msgPool, POOL_SIZE, sizeof(Msg_t) );
    task_create( button_task, 2, NULL, 0, 0 );
	...
}

 @endcode 
 *******************************************************************************/
#define task_resume(taskproc)     OS_RESUME_TASK( taskproc )


/*********************************************************************************/
/*  event_wait(event)                                                 *//**
*   
*   Macro for wait for a single event.
*
*	@param event: the event to wait for
*
*	@remarks \b Usage: @n 
* @code 
Evt_t myEvent;
main() {
 ...
 myEvent = event_create();
 ...
}

static void myTask(void) {
 task_open();	
  ...
  event_wait( myEvent );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define event_wait(event)    OS_WAIT_SINGLE_EVENT(event)


/*********************************************************************************/
/*  event_wait_multiple(waitAll, args...)                                                 *//**
*   
*   Macro for wait for multiple events.
*
*	@param waitAll 1 if wait for all, 0 if wait for any event
*   @param args list of Evt_t type events
*
*	@remarks \b Usage: @n 
* @code 
Evt_t myEvent1;
Evt_t myEvent2;
Evt_t myEvent3;
main() {
 ...
 myEvent1 = event_create();
 myEvent2 = event_create();
 myEvent3 = event_create();
 ...
}

static void myTask(void) {
 task_open();	
  ...
  event_wait_multiple(1, myEvent1, myEvent2, myEvent3);
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define event_wait_multiple(waitAll, args...)   OS_WAIT_MULTIPLE_EVENTS( waitAll, args)


/*********************************************************************************/
/*  event_signal(event)                                                 *//**
*   
*   Macro for signalling an event.
*
*	@param event: the event to be signalled
*       
*
*	@remarks \b Usage: @n 
* @code 
Evt_t myEvent;
main() {
 ...
 myEvent = event_create();
 ...
}

static void myTask(void) {
 task_open();	
  ...
  event_signal( myEvent );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define event_signal(event) OS_SIGNAL_EVENT(event)


/*********************************************************************************/
/*  event_ISR_signal(event)                                                 *//**
*   
*   Macro for signalling an event from an ISR
*
*	@param event: the event to be signalled
*       
*
*	@remarks \b Usage: @n 
* @code 
Evt_t evRxChar;
main() {
 ...
 evRxChar = event_create();
 ...
}

ISR (SIG_UART_RECV) {   
    rx.data[ rx.head ] = UDR;
    event_ISR_signal( evRxChar );
}
 @endcode 
 *******************************************************************************/
#define event_ISR_signal(event) OS_INT_SIGNAL_EVENT(event)


/*********************************************************************************/
/*  sem_wait(sem)                                                 *//**
*   
*   Macro for aquiring a semaphore.
*
*		@param sem Semaphore.
*
*		@remarks \b Usage: @n 
* @code 
Sem_t mySem;
main() {
 ...
 mySem = sem_create( 0 );
 ...
}

static void myTask(void) {
 task_open();	
  ...
  sem_wait( mySem );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define sem_wait(sem)  OS_WAIT_SEM(sem)


/*********************************************************************************/
/*  sem_signal(sem)                                                 *//**
*   
*   Macro for releasing a semaphore.
*
*		@param sem Semaphore.
*
*		@remarks \b Usage: @n 
* @code 
Sem_t mySem;
main() {
 ...
 mySem = sem_create( 0 );
 ...
}

static void myTask(void) {
 task_open();	
  ...
  sem_signal( mySem );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define sem_signal(sem)     OS_SIGNAL_SEM(sem)


/*********************************************************************************/
/*  msg_post(task, msg)                                            *//**
*   
*   Posts a message to the message queue of a task.
*
*		@param task task procedure of the task that will receive the message
*       @param msg the message to post
*		@return None
*		@remarks Before posting a message, the queue semaphore should be aquired with
*       a call to msg_q_get(). When the message has been posted, the semaphore should be 
*       released with msg_q_give(). If the message queue is full, the task will release the 
*       semaphore and wait for the queue change event to be signaled by another task. When
*       the event is signaled, the task will again aquire the semaphore and try to post the message.@n
*       \b Usage: @n 
* @code

typedef struct {
    Msg_t super;
    uint8_t led;
} LedMsg_t;

 
static LedMsg_t msgpool_1[ 16 ];


int main(void)
{
    ...
    task_create( task1, 1, NULL, 0, 0 );
    task_create( task2, 2, msgpool_1, 16, sizeof(LedMsg_t) );
    ...
    os_start();
    return 0;
}


static void task1(void) {
    static LedMsg_t ledMsg;

    ledMsg.super.signal = LED_SIG;
    ledMsg.led = 5;
    
    task_open();
    
    for (;;) {
        
        task_wait( 3000 );
        
        msg_q_get( task2 );

        msg_post( task2, ledMsg );

        msg_q_give( task2 );  
    }

    task_close();

}
 @endcode 
*		

*       
*/
/*********************************************************************************/
#define msg_post(task, msg)   OS_MSG_Q_POST(task, msg, 0, 0)


/*********************************************************************************/
/*  msg_post_in(task, msg, delay)                                            *//**
*   
*   Posts a message to the message queue of a task. The message will be received
*   by the receiver task when the delay has expired. The delay is related to the master
*   clock.
*
*		@param task task procedure of the task that will receive the message
*       @param msg the message to post
*       @param delay post delay time in master clock ticks
*		@return None
*		@remarks Before posting a message, the queue semaphore should be aquired with
*       a call to msg_q_get(). When the message has been posted, the semaphore should be 
*       released with msg_q_give(). If the message queue is full, the task will release the 
*       semaphore and wait for the queue change event to be signaled by another task. When
*       the event is signaled, the task will again aquire the semaphore and try to post the message.@n
*       \b Usage: @n 
* @code

typedef struct {
    Msg_t super;
    uint8_t led;
} LedMsg_t;

 
static LedMsg_t msgpool_1[ 16 ];


int main(void)
{
    ...
    task_create( task1, 1, NULL, 0, 0 );
    task_create( task2, 2, msgpool_1, 16, sizeof(LedMsg_t) );
    ...
    os_start();
    return 0;
}


static void task1(void) {
    static LedMsg_t ledMsg;

    ledMsg.super.signal = LED_SIG;
    ledMsg.led = 5;
    
    task_open();
    
    for (;;) {
        
        task_wait( 3000 );
        
        msg_q_get( task2 );

        msg_post_in( task2, ledMsg, 100 );

        msg_q_give( task2 );  
    }

    task_close();

}
 @endcode 
*		

*       
*/
/*********************************************************************************/
#define msg_post_in(task, msg, delay)   OS_MSG_Q_POST(task, msg, delay, 0)


/*********************************************************************************/
/*  msg_post_every(task, msg, period)                                            *//**
*   
*   Posts a periodic message to the message queue of a task. The message will be delivered
*   to the receiver task each time the timer expires. The period is related to the master
*   clock.
*
*		@param task task procedure of the task that will receive the message
*       @param msg the message to post
*       @param period period time in master clock ticks
*		@return None
*		@remarks Before posting a message, the queue semaphore should be aquired with
*       a call to msg_q_get(). When the message has been posted, the semaphore should be 
*       released with msg_q_give(). If the message queue is full, the task will release the 
*       semaphore and wait for the queue change event to be signaled by another task. When
*       the event is signaled, the task will again aquire the semaphore and try to post the message.@n
*       \b Usage: @n 
* @code

typedef struct {
    Msg_t super;
    uint8_t led;
} LedMsg_t;

 
static LedMsg_t msgpool_1[ 16 ];


int main(void)
{
    ...
    task_create( task1, 1, NULL, 0, 0 );
    task_create( task2, 2, msgpool_1, 16, sizeof(LedMsg_t) );
    ...
    os_start();
    return 0;
}


static void task1(void) {
    static LedMsg_t ledMsg;

    ledMsg.super.signal = LED_SIG;
    ledMsg.led = 5;
    
    msg_q_get( task2 );

    msg_post_every( task2, ledMsg, 100 );

    msg_q_give( task2 );
  
    task_open();
    
    for (;;) {
        ...
        ...
        task_wait( 3000 );
    }

    task_close();

}
 @endcode 
*		

*       
*/
/*********************************************************************************/
#define msg_post_every(task, msg, period)   OS_MSG_Q_POST(task, msg, period, period)


/*********************************************************************************/
/*  msg_receive( task, pMsg )                                            *//**
*   
*   Receives a message from the queue 
*
*       @param task task procedure of the current task
*		@param pMsg pointer to a message that will receive a message from the queue
*
*		@return None
*		@remarks Before receiving a message, the queue semaphore should be aquired with
*       a call to msg_q_get(). When the message has been received, the semaphore should be 
*       released with msg_q_give(). If the message queue is empty, the task will release the 
*       semaphore and wait for the queue change event to be signaled by another task. When
*       the event is signaled, the task will again aquire the semaphore and try to receive a message.@n
*       \b Usage: @n 
* @code

typedef struct {
    Msg_t super;
    uint8_t led;
} LedMsg_t;

 
static LedMsg_t msgpool_1[ 16 ];


int main(void)
{
    ...
    task_create( task1, 1, NULL, 0 );
    task_create( task2, 2, msgpool_1, 16, sizeof(LedMsg_t) );
    ...
    os_start();
    return 0;
}


static void task2(void) {
    static LedMsg_t msg;
    uint8_t led;
    task_open();
    
    for (;;) {
        
        task_wait( 3000 );
        
        msg_q_get( task2 );

        msg_receive( task2, &msg );

        msg_q_give( task2 );

        if ( msg.super.signal == LED_SIG ) {
            led = msg.led;
            LED_TOGGLE( led );
        } 
    }

    task_close();

}
 @endcode 
*       
*/
/*********************************************************************************/
#define msg_receive( task, pMsg )        OS_MSG_Q_RECEIVE( task, pMsg )



/*********************************************************************************/
/*  msg_q_get(task)                                                 *//**
*   
*   Macro for aquiring a message queue.
*
*		@param task Task procedure associated with the message queue
*
*		@remarks \b Usage: Should be called before any access to a message queue.
*       See msg_post() example @n 

 *******************************************************************************/
#define msg_q_get( task )           OS_MSG_Q_GET( task )

/*********************************************************************************/
/*  msg_q_give(task)                                                 *//**
*   
*   Macro for releasing a message queue.
*
*		@param task Task procedure associated with the message queue
*
*		@remarks \b Usage: Should be called after access to a message queue.
*       See msg_post() example @n 

 *******************************************************************************/
#define msg_q_give( task )          OS_MSG_Q_GIVE( task )





/*********************************************************************************/
/*  task_open()                                                 *//**
*   
*   Macro for definition of task beginning.
*
*		
*
*		@remarks \b Usage: Should be placed at the beginning of the task procedure @n 
* @code 


static void myTask(void) {
 static uint8_t i;
 task_open();	
  ...
  task_wait( 10 );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define task_open()                 OS_BEGIN


/*********************************************************************************/
/*  task_close()                                                 *//**
*   
*   Macro for definition of task end.
*
*		
*
*		@remarks \b Usage: Should be placed at the end of the task procedure @n 
* @code 


static void myTask(void) {
 static uint8_t i;
 task_open();	
  ...
  task_wait( 10 );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define task_close()                OS_END


/*********************************************************************************/
/*  task_wait(x)                                                 *//**
*   
*   Macro for suspending a task a specified amount of ticks of the master clock.
*   When the wait time has expired, the task is ready to execute again and will continue
*   at the next statement when the task is scheduled to run.
*
*		@param x Number of master clock ticks to wait.
*
*		@remarks \b Usage: @n 
* @code 


static void myTask(void) {
 task_open();	
  ...
  task_wait( 10 );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define task_wait(x)                OS_WAIT_TICKS(x,0)


/*********************************************************************************/
/*  task_wait_id(id,x)                                                 *//**
*   
*   Macro for suspending a task a specified amount of ticks of a sub clock.
*   When the wait time has expired, the task is ready to execute again and will continue
*   at the next statement when the task is scheduled to run.
*
*       @param id Sub clock id. Valid range 1-255.		
*       
*       @param x Number of sub clock ticks to wait, 16 bit value.
*		
*		@remarks \b Usage: @n 
* @code 


static void myTask(void) {
 task_open();	
  ...
  task_wait_id( 2, 10 );
  ...
 task_close();
}
 @endcode 
 *******************************************************************************/
#define task_wait_id(id,x)                OS_WAIT_TICKS(x,id)


void os_init( void );
void os_start( void );
void os_tick( void );
void os_sub_tick( uint8_t id );
void os_sub_nTick( uint8_t id, uint16_t nTicks );


uint8_t     task_create( taskproctype taskproc, uint8_t prio, Msg_t* msgPool, uint8_t poolSize, uint16_t msgSize );
TaskState_t task_state_get( taskproctype taskproc );
uint8_t     task_id_get( taskproctype taskproc );
void        task_kill( taskproctype taskproc );

Sem_t sem_bin_create( uint8_t initial );
Sem_t sem_counting_create( uint8_t max, uint8_t initial );

Evt_t   event_create( void );
uint8_t event_signaling_taskId_get( Evt_t ev );

void os_cbkSleep( void );



#endif
