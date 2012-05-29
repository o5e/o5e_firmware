/*
***************************************************************************************
***************************************************************************************
***
***     File: os_assert.h
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
    2010-07-14: 1.0.0 First release
    


***************************************************************************************
*/

#ifndef _os_assert_h__
#define _os_assert_h__

#include <inttypes.h>

void os_on_assert( uint16_t line );

#ifndef NASSERT
#define os_assert( test )   if ( !(test) ) {\
                                os_on_assert(__LINE__);\
                            }
#else
#define os_assert( test )
#endif


#endif