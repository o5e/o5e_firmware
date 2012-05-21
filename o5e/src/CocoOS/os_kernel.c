/*
***************************************************************************************
***************************************************************************************
***
***     File: os_kernel.c
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
	2011-12-04: Calling the sleep callback when all tasks waiting.
	2011-12-08: Implemented os_sub_tick
	2011-12-14: Added os_sub_nTick, for incrementing sub clocks in steps > 1


***************************************************************************************
*/



#include "cocoos.h"


uint8_t running_tid;
uint8_t running;

/*********************************************************************************/
/*  void os_init()                                              *//**
*   
*   Initializes the scheduler.
*
*		@return None.
*
*		@remarks \b Usage: @n Should be called early in system setup, before starting the task 
*       execution
*	
*		
*       @code
int main(void) {
	system_init();
	os_init();
	...
}
*		@endcode
*       
*		 */
/*********************************************************************************/
void os_init( void ) {
	running_tid = NO_TID;
    running = 0;
}




void os_schedule( void ) {

    running_tid = NO_TID;

    /* Find the highest prio task ready to run */
	running_tid = os_task_highest_prio_ready_task();
	
	if ( running_tid != NO_TID) {
        os_task_run_highest();
	}
    else {
        os_cbkSleep();
    }
    
}




/*********************************************************************************/
/*  void os_start()                                              *//**
*   
*   Starts the task scheduling
*
*
*		@return None.
*
*		@remarks \b Usage: @n Should be the last line of main.
*
*	
*		
*       @code
int main(void) {
	system_init();
	os_init();
	task_create( myTaskProc, 1, NULL, 0, 0 );
	...
	os_start();
	return 0;
}
*		@endcode
*       
*/
/*********************************************************************************/
void os_start( void ) {
    running = 1;
	os_enable_interrupts();

	for (;;){
		os_schedule();
	}
}



/*********************************************************************************/
/*  void os_tick()                                              *//**
*   
*   Tick function driving the kernel
*
*
*		@return None.
*
*		@remarks \b Usage: @n Should be called periodically. Preferably from the clock tick ISR.
*
*	
*		
*
*       @code
ISR(SIG_OVERFLOW0) {
	...
    os_tick();	
}
*		@endcode
*       
*/
/*********************************************************************************/
void os_tick( void ) {
    /* Master clock tick */
    os_task_tick( 0, 1 );
}


/*********************************************************************************/
/*  void os_sub_tick( id )                                              *//**
*   
*   Tick function driving the sub clocks
*
*       @param id sub clock id, allowed range 1-255
*
*		@return None.
*
*		@remarks \b Usage: @n Could be called at any desired rate to trigger timeouts. @n
*Called from a task or from an interrupt ISR.
*
*	
*		
*
*       @code
ISR(SIG_OVERFLOW0) {
	...
    os_sub_tick(2);	
}
*		@endcode
*       
*/
/*********************************************************************************/
void os_sub_tick( uint8_t id ) {
    /* Sub clock tick */
    if ( id != 0 ) {
        os_task_tick( id, 1 );
    }
}


/*********************************************************************************/
/*  void os_sub_nTick( id, nTicks )                                              *//**
*   
*   Tick function driving the sub clocks. Increments the tick count with nTicks.
*
*       @param id sub clock id, allowed range 1-255.
*       @param nTicks increment size, 16 bit value.
*
*		@return None.
*
*		@remarks \b Usage: @n Could be called at any desired rate to trigger timeouts. @n
*Called from a task or from an interrupt ISR.
*
*	
*		
*
*       @code
ISR(SIG_OVERFLOW0) {
	...
    os_sub_nTick( 2, 10 );	
}
*		@endcode
*       
*/
/*********************************************************************************/
void os_sub_nTick( uint8_t id, uint16_t nTicks ) {
    /* Sub clock tick */
    if ( id != 0 ) {
        os_task_tick( id, nTicks );
    }
}


uint8_t os_running( void ) {
    return running;
}

