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
 
/***************************************************************************************


    Change log:
    2010-10-20: 1.0.0 File creation
    2010-10-23: Added the enum.
    2010-11-17: New macros for posting and receiving messages which takes care of releasing
                the semaphore and waiting for the event in case of full/empty buffer.

    2011-01-03: Changed the macros for posting and receiving messages to support delayed
                messaging.

    2011-01-13: Changed the macor for posting messages to support periodic messages.
    
    2012-01-04: Released under BSD license.

***************************************************************************************
*/

#ifndef OS_MSGQUEUE_H__
#define OS_MSGQUEUE_H__

/** @file os_msgqueue.h Message queue header file*/

#include "os_defines.h"


typedef struct {
    uint8_t signal;
    uint8_t reserved;   /* Alignment byte */
    uint16_t delay;     /* Delay of posting in ticks */
    uint16_t reload;    /* Reload value for periodic messages */
} Msg_t;


typedef uint8_t MsgQ_t;


enum {
    MSG_QUEUE_UNDEF,
    MSG_QUEUE_DEF,
    MSG_QUEUE_EMPTY,
    MSG_QUEUE_FULL,
    MSG_QUEUE_RECEIVED,
    MSG_QUEUE_POSTED
};



#define OS_MSG_Q_POST(task, msg, delay, period)   do {\
                                        static uint8_t os_posted;\
                                        static Sem_t sem;\
                                        do {\
                                            os_posted = os_msg_post( task, (Msg_t*)&msg, delay, period );\
                                            if ( os_posted == MSG_QUEUE_FULL ){\
                                                sem = os_msgQ_sem_get(task);\
                                                OS_SIGNAL_SEM_NO_SCHEDULE(sem);\
                                                event_wait(os_msgQ_event_get(task));\
							                    if ( os_sem_larger_than_zero( sem )  )\
						  		                    os_sem_decrement( sem );\
							                    else {\
								                    os_task_wait_sem_set( running_tid, sem );\
								                    os_task_internal_state_set(running_tid, __LINE__+50000);\
                                                    running_tid = NO_TID;\
					                                return;\
					                                case (__LINE__+50000):\
                                                    continue;\
						  	                    }\
                                            }\
                                        } while ( os_posted == MSG_QUEUE_FULL );\
                                    } while(0)



#define OS_MSG_Q_RECEIVE(task, pMsg)   do {\
                                            static uint8_t os_received;\
                                            static Sem_t sem;\
                                            do {\
                                                os_received = os_msg_receive( (Msg_t*)pMsg );\
                                                if ( os_received == MSG_QUEUE_EMPTY ){\
                                                    sem = os_msgQ_sem_get(task);\
                                                    OS_SIGNAL_SEM_NO_SCHEDULE(sem);\
                                                    event_wait(os_msgQ_event_get(task));\
							                        if ( os_sem_larger_than_zero( sem )  )\
						  		                        os_sem_decrement( sem );\
							                        else {\
								                        os_task_wait_sem_set( running_tid, sem );\
								                        os_task_internal_state_set(running_tid, __LINE__+50000);\
                                                        running_tid = NO_TID;\
					                                    return;\
					                                    case (__LINE__+50000):\
                                                        continue;\
						  	                        }\
                                                }\
                                            } while ( os_received == MSG_QUEUE_EMPTY );\
                                        } while(0)

#define OS_MSG_Q_GET( task )        sem_wait( os_msgQ_sem_get( task ) )

#define OS_MSG_Q_GIVE( task )       sem_signal( os_msgQ_sem_get( task ) )

#define OS_MSG_Q_EVENT_WAIT( task ) do {\
                                        OS_SIGNAL_SEM_NO_SCHEDULE(os_msgQ_sem_get(task));\
                                        event_wait(os_msgQ_event_get(task));\
                                    } while (0) 



MsgQ_t os_msgQ_create( Msg_t *buffer, uint8_t size, uint16_t msgSize );
uint8_t os_msg_post( taskproctype taskproc, Msg_t *msg, uint16_t delay, uint16_t period );
uint8_t os_msg_receive( Msg_t *msg );
Sem_t os_msgQ_sem_get( taskproctype taskproc );
Evt_t os_msgQ_event_get( taskproctype taskproc );
void os_msgQ_tick( MsgQ_t queue );

#endif
