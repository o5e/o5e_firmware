/*********************************************************************************

    @file      Base_Values_OPS.c                                                              
    @date      April 2013
    @brief     Open5xxxECU - this file contains functions for calculating base values where needed
    @note      www.Open5xxxECU.org
    @version   2.2
    @copyright 2013 - M. Eberhardt

**********************************************************************************/

// Portions Copyright 2013 M. Eberhardt - BSD 3 clause License


#include <stdint.h>
#include "config.h"
#include "err.h"
#include "variables.h"
#include "typedefs.h"
#include "Base_Values_OPS.h"
#include "Table_Lookup.h"

uint32_t Base_Pulse_Width;
uint32_t Inverse_Injector_Pressure; //bin 20
uint32_t Injector_Flow;
  
void Get_Base_Pulse_Width(void)
{  
     
	//correct injector flow rate for actual fuel pressure
	Inverse_Injector_Pressure = (1 <<20) / (uint32_t )Rating_Fuel_Presure;
    Injector_Flow = (Fuel_Presure * Inverse_Injector_Pressure) >>10; //convert to bin 10
    Injector_Flow = (uint32_t)(table_lookup(((int16_t)Injector_Flow), 0, sqrt_Table));
    Injector_Flow = (Injector_Flow * Injector_Size) >>10;// get current injector folw and convert to bin 0	
	
    // base (max) pulse width
     Base_Pulse_Width = (uint32_t)(Displacement / N_Cyl);

     Base_Pulse_Width = ((Base_Pulse_Width *(uint32_t) gram_STP_Air_Per_cc) >> 10); //convert displacement in cc to g and convert bin 24 to bin 14
     Base_Pulse_Width = (Base_Pulse_Width * (uint32_t)Base_Air_Fuel_Ratio) >>12; // get g/min and convert bin14 to  bin 0 
     Base_Pulse_Width = (Base_Pulse_Width * (uint32_t)Gasoline_SG) >>8; //get volumetric fuel required and convert bin 12 to bin 4
     Base_Pulse_Width = (Base_Pulse_Width / Injector_Flow); //convert to min and convert bin 10 to bin 2
     //Max_Inj_Time
     Base_Pulse_Width = (Base_Pulse_Width* ((1000000 >>8) * 60)) >>12; //convert to usec and convert bin 2 to bin 0         	    		 
}//Get_Base_Pulse_Width


