/*
***************************************************************************************
***************************************************************************************
***
***     File: os_msgqueue.c
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
    2010-10-20: 1.0.0: File creation
    2010-10-23: Added function for checking for empty queue.
                Head and tail pointers counts down instead of up. This makes wrap-around
                check easier.
                Error checks when posting and receiving messages.
    
    2010-10-25: The semaphore and change event was added.
	
    2010-10-26: The change event is signaled when receiving and posting a message

    2010-11-18: Messages are stored as messages in the queue, not as message pointers.
                So, when posting and receiving messages, byte copying is done. The size
                of the messsage type held in the queue is added as a member of the OSMsgQ_t.
                
    2011-01-03: Implemented delayed message posting. A message can be posted with a 
                specified delay. This means the message will be delivered to a message 
                receive operation only when the timer has expired. If the message at the 
                tail position has not expired, it will be removed from the queue and put
                back at the head position.

    2011-01-13: Support for periodic messages. A message that is posted with post_every()
                will be delivered to the receiver periodically. The message is constantly
                reposted into the queue with the delay time reloaded.
***************************************************************************************
*/



#include "cocoos.h"


typedef struct {
    Msg_t *msgPool;
    uint8_t head;
    uint8_t tail;
    uint8_t size;
    Sem_t mutex;
    Evt_t change;
    uint16_t messageSize;
} OSMsgQ_t;






#if( N_QUEUES > 0 )
static uint8_t MsgQEmpty( OSMsgQ_t *q );
static uint8_t MsgQFull( OSMsgQ_t *q );
static uint8_t TaskMsgQGet( taskproctype taskproc, MsgQ_t *queue );
static uint8_t MsgQAllDelayed( OSMsgQ_t *q );

/* List of task message queues */
static OSMsgQ_t msgQList[ N_QUEUES ];
static MsgQ_t nQueues = 0;
#endif


MsgQ_t os_msgQ_create( Msg_t *buffer, uint8_t size, uint16_t msgSize ) {
#if( N_QUEUES > 0 )
    os_assert( nQueues < N_QUEUES );
    msgQList[ nQueues ].msgPool = buffer;
    msgQList[ nQueues ].head = size - 1;
    msgQList[ nQueues ].tail = size - 1;
    msgQList[ nQueues ].size = size;
    msgQList[ nQueues ].mutex = sem_bin_create( 1 );
    msgQList[ nQueues ].change = event_create();
    msgQList[ nQueues ].messageSize = msgSize;
    nQueues++;
    return nQueues - 1;
#else
    return 0;
#endif
}


Sem_t os_msgQ_sem_get( taskproctype taskproc ) {
#if( N_QUEUES > 0 )
    uint8_t result;
    
    MsgQ_t queue;
        
    result = TaskMsgQGet( taskproc, &queue );
        
    if ( result == MSG_QUEUE_UNDEF ) {
        return NO_SEM;
    }

    return msgQList[ queue ].mutex;
#else
    return 0;
#endif
}


Evt_t os_msgQ_event_get( taskproctype taskproc ) {
#if( N_QUEUES > 0 )
    uint8_t result;
    
    MsgQ_t queue;
        
    result = TaskMsgQGet( taskproc, &queue );
        
    if ( result == MSG_QUEUE_UNDEF ) {
        return NO_EVENT;
    }

    return msgQList[ queue ].change;
#else
    return 0;
#endif
}


uint8_t os_msg_post( taskproctype taskproc, Msg_t *msg, uint16_t delay, uint16_t period ) {
#if( N_QUEUES > 0 )
    OSMsgQ_t *temp;
    MsgQ_t queue;
    uint8_t result;
    uint8_t *data;
    uint8_t i;
        
    result = TaskMsgQGet( taskproc, &queue );

    if ( result == MSG_QUEUE_UNDEF ) {
        return result;
    }

    temp = &msgQList[ queue ];

    if ( MsgQFull( temp ) == 1 ) {
        return MSG_QUEUE_FULL;
    }

    data = (uint8_t*)( (uint16_t)temp->msgPool + (temp->head) * (temp->messageSize) );
    
    msg->delay = delay;
    msg->reload = period;

    for ( i = 0; i < temp->messageSize; ++i ) {
        data[ i ] = ((uint8_t*)msg)[i];
    }
    
        
    /* Look for buffer wrap around */
    if ( temp->head == 0 ) {
        temp->head = temp->size - 1;
    }
    else {
        temp->head--;
    }

    /* Signal the change event */
    os_signal_event(temp->change);
    os_event_set_signaling_tid( temp->change, running_tid );

    return MSG_QUEUE_POSTED;
#else
    return 0;
#endif
}



uint8_t os_msg_receive( Msg_t *msg ) {
#if( N_QUEUES > 0 )
    MsgQ_t queue;
    OSMsgQ_t *temp;
    uint8_t *data;
    uint8_t i;
    uint8_t found;

    /* Get the queue belonging to the running task */
    queue = os_task_msgQ_get( running_tid );

    if ( queue == NO_QUEUE ) {
        return MSG_QUEUE_UNDEF;
    }

    temp = &msgQList[ queue ];

    if ( MsgQEmpty( temp ) == 1 ) {
        return MSG_QUEUE_EMPTY;
    }

    /* If all messages have a delay > 0 we consider the queue as empty */
    if ( MsgQAllDelayed( temp ) == 1 ) {
        return MSG_QUEUE_EMPTY;
    }

    /* At least one message is ready to be delivered. Find it! */
    found = 0;
    while ( found == 0 ) {
        uint8_t messagePeriodic;
        uint8_t messageTimedOut;

        data = (uint8_t*)( (uint16_t)temp->msgPool + (temp->tail) * (temp->messageSize) );
        for ( i = 0; i < temp->messageSize; ++i ) {
            ((uint8_t*)msg)[ i ] = data[ i ];
        }
    

        /* Look for buffer wrap around */
        if ( temp->tail == 0 ) {
            temp->tail = temp->size - 1;
        }
        else {
            temp->tail--;
        }

        messagePeriodic = ( msg->reload > 0 );
        messageTimedOut = ( msg->delay == 0 );

        if ( messageTimedOut ){
            found = 1;
            if ( messagePeriodic ) {
                msg->delay = msg->reload;
            }
        }

        /* Put the message back at head position if delay > 0, or if it is a periodic message that timed out */
        if (( !messageTimedOut ) || ( messagePeriodic && messageTimedOut )) {
            data = (uint8_t*)( (uint16_t)temp->msgPool + (temp->head) * (temp->messageSize) );
            
            for ( i = 0; i < temp->messageSize; ++i ) {
                data[ i ] = ((uint8_t*)msg)[i];
            }
    
        
            /* Look for buffer wrap around */
            if ( temp->head == 0 ) {
                temp->head = temp->size - 1;
            }
            else {
                temp->head--;
            }
        }
        
        
    }
    /* Signal the change event */
    os_signal_event(temp->change);
    os_event_set_signaling_tid( temp->change, running_tid );
    
    return MSG_QUEUE_RECEIVED;
#else
    return 0;
#endif
}


void os_msgQ_tick( MsgQ_t queue ) {
#if( N_QUEUES > 0 )
    uint16_t nextMessage;
    Msg_t *pMsg;
    OSMsgQ_t *temp = &msgQList[ queue ];

    nextMessage = temp->tail;

    while ( nextMessage != temp->head ) {
        pMsg = (Msg_t*)( (uint16_t)temp->msgPool + (nextMessage) * (temp->messageSize) );
        if ( pMsg->delay > 0 ) {
            --(pMsg->delay);
            if ( pMsg->delay == 0 ) {
                event_ISR_signal( temp->change );
            }
        }
        if ( nextMessage == 0 ) {
            nextMessage = temp->size - 1;
        }
        else {
            --nextMessage;
        }
    }
#endif    
}

#if( N_QUEUES > 0 )
static uint8_t MsgQEmpty( OSMsgQ_t *q ) {
    return ( q->head == q->tail );
}


static uint8_t MsgQAllDelayed( OSMsgQ_t *q ) {
    uint8_t nextMessage;
    Msg_t *pMsg;
    uint8_t result;

    result = 1;
    nextMessage = q->tail;

    while ( nextMessage != q->head ) {
        pMsg = (Msg_t*)( (uint16_t)q->msgPool + (nextMessage) * (q->messageSize) );
        
        if ( pMsg->delay == 0 ) {
            result = 0;
            break;
        }

        if ( nextMessage == 0 ) {
            nextMessage = q->size - 1;
        }
        else {
            --nextMessage;
        }
    }
    return result;

}


static uint8_t MsgQFull( OSMsgQ_t *q ) {
    uint8_t nextHead;

    nextHead = q->head;

    if ( nextHead == 0 ) {
        nextHead = q->size - 1;
    }
    else {
        nextHead--;
    }

    return ( nextHead == q->tail );
}


static uint8_t TaskMsgQGet( taskproctype taskproc, MsgQ_t *queue ) {
    uint8_t taskId;
    
    /* Find the queue belonging to this task */
    taskId = task_id_get( taskproc );

    if ( taskId == NO_TID ) {
        return MSG_QUEUE_UNDEF;
    }

    *queue = os_task_msgQ_get( taskId );

    if ( *queue == NO_QUEUE ) {
        return MSG_QUEUE_UNDEF;
    }

    return MSG_QUEUE_DEF;
    
}

#endif
