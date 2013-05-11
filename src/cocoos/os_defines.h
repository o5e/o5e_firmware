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
    2009-07-06: 1.0.0 First release
    2010-06-13: Removed TRUE and FALSE definitions and Bool typedef
    added os_assert macro
    
    2010-10-23: NO_QUEUE macro
    2010-10-26: N_USER_SEMAPHORES and N_USER_EVENTS macros
    2010-11-01: Moved some macros to cocoos.h
    2012-01-04: Released under BSD license.


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
#define N_QUEUES            0


/** Max number of used semaphores
* @remarks Must be defined. @n Allowed range: 0-254. Value must not be exceeded */
#define N_SEMAPHORES        0


/** Max number of used events
* @remarks Must be defined. @n Allowed range: 0-254. Value must not be exceeded */
#define N_EVENTS            0


/** Round Robin scheduling
* @remarks If defined, tasks will be scheduled ignoring the priorities */
//#define ROUND_ROBIN

#endif
