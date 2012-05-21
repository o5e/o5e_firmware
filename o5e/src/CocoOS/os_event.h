/*
***************************************************************************************
***************************************************************************************
***
***     File: os_event.h
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
    2010-10-12: New event type definition


***************************************************************************************
*/
#ifndef OS_EVENT_H
#define OS_EVENT_H

/** @file os_event.h Event header file*/

#include "cocoos.h"
#include "stdarg.h"




#define OS_WAIT_SINGLE_EVENT(x)	do {\
								os_wait_event(running_tid,x,1);\
								OS_SCHEDULE;\
							   } while (0)






#define OS_WAIT_MULTIPLE_EVENTS( waitAll, args...)	do {\
								os_wait_multiple(waitAll, args, NO_EVENT);\
								OS_SCHEDULE;\
							   } while (0)






#define OS_SIGNAL_EVENT(event)	do {\
								os_signal_event(event);\
								os_event_set_signaling_tid( event, running_tid );\
								OS_SCHEDULE;\
								} while (0)




#define OS_INT_SIGNAL_EVENT(event)	do {\
									os_signal_event(event);\
									} while (0)


#ifdef N_TOTAL_EVENTS
    #define EVENT_QUEUE_SIZE    ((N_TOTAL_EVENTS/9)+1)
#else
    #define EVENT_QUEUE_SIZE    1
#endif


typedef uint8_t Evt_t;

typedef struct {
    uint8_t eventList[ EVENT_QUEUE_SIZE ];
} EventQueue_t;



void os_wait_event( uint8_t tid, Evt_t ev, uint8_t waitSingleEvent );
void os_wait_multiple( uint8_t waitAll, ...);
void os_signal_event( Evt_t ev );
void os_event_set_signaling_tid( Evt_t ev, uint8_t tid );



#endif
