/*
***************************************************************************************
***************************************************************************************
***
***     File: os_sem.h
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
    2010-07-20: When waiting for semaphore, a pointer to the semaphore is passed as
    a parameter for storing in the task tcb.

    2010-07-26: Adjusted the macros for the new semaphore handling. Renamed the semaphore
    type.


***************************************************************************************
*/
#ifndef OS_SEM_H
#define OS_SEM_H

/** @file os_sem.h Semaphore header file*/

#include "cocoos.h"



#define OS_WAIT_SEM(sem)		do {\
								if ( os_sem_larger_than_zero( sem )  )\
							  		os_sem_decrement( sem );\
								else\
							 	{\
									os_task_wait_sem_set( running_tid, sem );\
									OS_SCHEDULE;\
							  	}\
						       } while (0)


#define OS_WAIT_SEM_NO_SCHEDULE(sem)		do {\
								                if ( os_sem_larger_than_zero( sem )  ) {\
							  		                os_sem_decrement( sem );\
                                                }\
								                else {\
									                os_task_wait_sem_set( running_tid, sem );\
                                                }\
                                            } while (0)


#define OS_SIGNAL_SEM(sem) 	do {\
								    if ( os_task_waiting_this_semaphore( sem ) == 0 ) {\
								        os_sem_increment( sem );\
                                    }\
								    else {\
									    os_task_release_highest_prio_task( sem );\
									    OS_SCHEDULE;\
								    }\
							    } while (0)



#define OS_SIGNAL_SEM_NO_SCHEDULE(sem) 	do {\
								            if ( os_task_waiting_this_semaphore( sem ) == 0 ) {\
								                os_sem_increment( sem );\
                                            }\
								            else {\
									            os_task_release_highest_prio_task( sem );\
								            }\
							            } while (0)



typedef uint8_t Sem_t;


uint8_t os_sem_larger_than_zero( Sem_t sem );
void os_sem_decrement( Sem_t sem );
void os_sem_increment( Sem_t sem );


#endif
