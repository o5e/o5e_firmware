/*
***************************************************************************************
***************************************************************************************
***
***     File: os_defines.h
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
    2010-06-13: Removed TRUE and FALSE definitions and Bool typedef
    added os_assert macro
    
    2010-10-23: NO_QUEUE macro
    2010-10-26: N_USER_SEMAPHORES and N_USER_EVENTS macros
    2010-11-01: Moved some macros to cocoos.h


***************************************************************************************
*/
#ifndef _os_defs
#define _os_defs

/** @file os_defines.h cocoOS user configuration */




/** Max number of used tasks
* @remarks Must be defined. @n Allowed range: 0-254. Value must not be exceeded */
#define N_TASKS             20


/** Max number of used message queues
* @remarks Must be defined. @n Allowed range: 0-254. Value must not be exceeded */
#define N_QUEUES            2


/** Max number of used semaphores
* @remarks Must be defined. @n Allowed range: 0-254. Value must not be exceeded */
#define N_SEMAPHORES        2


/** Max number of used events
* @remarks Must be defined. @n Allowed range: 0-254. Value must not be exceeded */
#define N_EVENTS            3


#endif
