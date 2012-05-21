/*
***************************************************************************************
***************************************************************************************
***
***     File: os_cbk.c
***
***     Project: cocoOS
***
***     Copyright 2011 Peter Eckstrand
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


    

    Change log:
    2011-12-04: First release
	

***************************************************************************************
*/
#include "cocoos.h"

/************************************************************** *******************/
/*  void os_cbkSleep( void )    *//**
*   Callback called by the os kernel when all tasks are in waiting state. Here you 
*   can put the MCU to low power mode. Remember to keep the clock running so we can 
*	wake up from sleep.
*
*		
*       
*/
/*********************************************************************************/
void os_cbkSleep( void ) {
    /* Enter low power mode here */
}
