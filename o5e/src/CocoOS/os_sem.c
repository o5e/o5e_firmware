/*
***************************************************************************************
***************************************************************************************
***
***     File: os_sem.c
***
***     Project: cocoOS
***
***     Author: Peter Eckstrand
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
    2010-07-26: Removed the waiting list from the semaphore type. Renamed the semaphore 
                creating function.
    
    2010-10-06: Changed the semaphore type to be a uint8_t. semList is a list of all
                created semaphores. A semaphore is identified by the use of the index
                in the semList.
                
    2010-10-18: Added support for binary and counting semaphores             


***************************************************************************************
*/

#include "cocoos.h"


typedef struct {
    uint8_t maxValue;
    uint8_t value;    
} SemValue_t;


static Sem_t os_sem_create( uint8_t max, uint8_t initial );

#if ( N_TOTAL_SEMAPHORES > 0 )
static SemValue_t semList[ N_TOTAL_SEMAPHORES ];
static Sem_t nSemaphores = 0;
#endif							   
							   
static Sem_t os_sem_create( uint8_t max, uint8_t initial ) {
#if ( N_TOTAL_SEMAPHORES > 0 )
    os_assert( nSemaphores < N_TOTAL_SEMAPHORES );

    /* Initialize the value and the waiting list */
    semList[ nSemaphores ].maxValue = max;
    semList[ nSemaphores ].value = initial;
    ++nSemaphores;

    return ( nSemaphores - 1 );
#else
    return 0;
#endif
}


/*********************************************************************************/
/*  Sem_t sem_bin_create(uint8_t initial)                                              *//**
*   
*   Creates and initializes a new binary semaphore.
*
*		@param initial value of the semaphore
*
*		@return Returns the created semaphore.
*
*		@remarks \b Usage: @n A semaphore is created by declaring a variable of type Sem_t 
*       and then assigning the sem_create(value) return value to that variable.
*	
*		
*       @code
*       Sem_t mySem;
*       mySem = sem_bin_create(0);
*		@endcode
*       
*		 */
/*********************************************************************************/
Sem_t sem_bin_create( uint8_t initial ) {
    return os_sem_create( 1, initial );
}


/*********************************************************************************/
/*  Sem_t sem_counting_create(uint8_t max, uint8_t initial)                                              *//**
*   
*   Creates and initializes a new counting semaphore.
*
*		@param max value of the semaphore
*       @param initial value of the semaphore
*
*		@return Returns the created semaphore.
*
*		@remarks \b Usage: @n A semaphore is created by declaring a variable of type Sem_t 
*       and then assigning the sem_create(max, 0) return value to that variable.
*	
*		
*       @code
*       Sem_t mySem;
*       mySem = sem_counting_create(5,0);
*		@endcode
*       
*		 */
/*********************************************************************************/
Sem_t sem_counting_create( uint8_t max, uint8_t initial ) {
    return os_sem_create( max, initial );
}


uint8_t os_sem_larger_than_zero( Sem_t sem) {
#if ( N_TOTAL_SEMAPHORES > 0 )
    return ( semList[ sem ].value > 0 );
#else
    return 0;
#endif
}


void os_sem_decrement( Sem_t sem ) {
#if ( N_TOTAL_SEMAPHORES > 0 )
    if ( semList[ sem ].value > 0 ) {
        --semList[ sem ].value;
    }
#endif
}


void os_sem_increment( Sem_t sem ) {
#if ( N_TOTAL_SEMAPHORES > 0 )
    if ( semList[ sem ].value < semList[ sem ].maxValue ) {
        ++semList[ sem ].value;
    }
#endif
}



