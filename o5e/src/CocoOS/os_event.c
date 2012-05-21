/*
***************************************************************************************
***************************************************************************************
***
***     File: os_event.c
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

    2010-10-12: New event types. the Evt_t is just a uint8_t that the application
    uses when refering to an event. The event is just an index into the eventList.
    The user has to define the number of events used by the application with the N_EVENTS
    macro in os_defines.h. Event_t is a struct used by cocoOS when managing events.


***************************************************************************************
*/



#include "cocoos.h"
#include "stdarg.h"



/* Event type */
typedef struct {
		uint8_t id;
		uint8_t signaledByTid;
		} Event_t;



/* Event list */
#if( N_TOTAL_EVENTS > 0 )
static Event_t eventList[ N_TOTAL_EVENTS ];

/* Keeping track of number of created events */
static Evt_t nEvents = 0;
#endif

/*********************************************************************************/
/*  Evt_t event_create()                                              *//**
*   
*   Creates an event.
*
*		@return Returns an event.
*
*		@remarks \b Usage: @n An event is created by declaring a variable of type Evt_t and then
*		assigning the event_create() return value to that variable.
*	
*		
*       @code
*       Evt_t myEvent;
*       myEvent = event_create();
*		@endcode
*       
*		 */
/*********************************************************************************/

Evt_t event_create( void ) {
    #if( N_TOTAL_EVENTS > 0 )
	os_assert( nEvents < N_TOTAL_EVENTS );

    eventList[ nEvents ].id = nEvents;
    eventList[ nEvents ].signaledByTid = NO_TID;
	
	++nEvents;

	return nEvents - 1;
    #else
    return 0;
    #endif
}


/*********************************************************************************/
/*  uint8_t event_signaling_taskId_get( ev )                                              *//**
*   
*   Gets the Task Id of the task that signaled the event.
*
*   
*    @param ev event
*
*    @return Id of task that signaled the event.
*
*    @remarks \b Usage: @n A task can make a call to this function when it has resumed
execution after waiting for an event to find out which other task signaled the event.
*	
*		
*       @code
*       event_wait(event);
*       signalingTask = event_signaling_taskId_get(event);
*       if ( signalingTask == Task2_id ) {
*         ...
*       }
*		@endcode		
*       
*		 */
/*********************************************************************************/
uint8_t event_signaling_taskId_get( Evt_t ev ) {
#if( N_TOTAL_EVENTS > 0 )
	return eventList[ ev ].signaledByTid;
#else
    return 0;
#endif
}


void os_wait_event(uint8_t tid, Evt_t ev, uint8_t waitSingleEvent) {
#if( N_TOTAL_EVENTS > 0 )
    if ( ev < nEvents ) {
        os_task_wait_event( tid, ev, waitSingleEvent );
    }
#endif
}


void os_signal_event( Evt_t ev ) {
    os_task_signal_event( ev );
}


void os_event_set_signaling_tid( Evt_t ev, uint8_t tid ) {
#if( N_TOTAL_EVENTS > 0 )
	eventList[ ev ].signaledByTid = tid;
#endif
}


void os_wait_multiple( uint8_t waitAll, ...) {
#if( N_TOTAL_EVENTS > 0 )
	int event;
	va_list args;
	va_start( args, waitAll );
	os_task_clear_wait_queue( running_tid );
	event = va_arg( args, int );

	do {
		os_task_wait_event( running_tid, (Evt_t)event, !waitAll );
		event = va_arg( args, int );
	} while ( event != NO_EVENT );

	va_end(args);
#endif
}



