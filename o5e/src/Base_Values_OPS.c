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

float Base_Pulse_Width;
float Inverse_Injector_Pressure;
float Injector_Flow;
  
void Get_Base_Pulse_Width(void)
{  
     
	//correct injector flow rate for actual fuel pressure
	Inverse_Injector_Pressure = 1 / Rating_Fuel_Presure;
    Injector_Flow = Fuel_Presure * Inverse_Injector_Pressure; 
    Injector_Flow = table_lookup(Injector_Flow, 0, sqrt_Table);
    Injector_Flow = Injector_Flow * Injector_Size;// get current injector folw	
	
    // base (max) pulse width
     Base_Pulse_Width = Displacement / ((float)N_Cyl);

     Base_Pulse_Width = Base_Pulse_Width * gram_STP_Air_Per_cc; //convert displacement in cc to g
     Base_Pulse_Width = Base_Pulse_Width * Base_Air_Fuel_Ratio; // get g/min 
     Base_Pulse_Width = Base_Pulse_Width * Gasoline_SG; //get volumetric fuel required
     Base_Pulse_Width = Base_Pulse_Width / Injector_Flow;
     //Max_Inj_Time
     Base_Pulse_Width = Base_Pulse_Width * 1000000 * 60 ; //convert to usec         	    		 
}//Get_Base_Pulse_Width


