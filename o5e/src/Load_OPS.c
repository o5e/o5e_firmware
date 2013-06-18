/*********************************************************************************

    @file      Load_OPS.c                                                              
    @date      April 2013
    @brief     Open5xxxECU - this file contains functions load calculation
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
#include "Load_OPS.h"
#include "Base_Values_OPS.h"
#include "Table_Lookup.h"

float gram_flow;
  
void Get_Reference_VE(void)
{  
     
  if (Load_Sense <= 3){
      Reference_VE = MAP[1];//  using MAP 2 until angle reading fixed on MAP1 (MAP[0])
      // Air temperature correction....I can't figure out how to not make this a divide at the moment
      Reference_VE = Reference_VE  / IAT;	
  }else if (Load_Sense == 4){
      Reference_VE = TPS * MAP[1];
      //correct for TPS flow if used.
      if (TPS_Flow_Cal_On == 1){
	  Reference_VE = Reference_VE  * table_lookup(RPM, TPS, TPS_Flow_Table);
      }//if
      //Air temperature correction....I can't figure out how to not make this a divide at the moment
      Reference_VE = Reference_VE  / IAT;	
  }else{	//(Load_Sense == 5, use MAF 
      gram_flow = gram_STP_Air_Per_cc * Displacement ; //convert displacement in cc to g
      gram_flow = gram_flow * (RPM / 2); // get g/min 
      Reference_VE = (MAF[0] * 60) / gram_flow ; //get %VE 

  }//if   	    		 
}//Get_Reference_VE


