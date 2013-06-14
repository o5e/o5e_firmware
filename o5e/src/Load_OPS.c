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

uint32_t gram_flow;
  
void Get_Reference_VE(void)
{  
     
  if (Load_Sense <= 3){
      Reference_VE = (int16_t)MAP[1];//  using MAP 2 until angle reading fixed on MAP1 (MAP[0])
      // Air temperature correction....I can't figure out how to not make this a divide at the moment
      Reference_VE = (Reference_VE << 14) / (int16_t)IAT;	
  }else if (Load_Sense == 4){
      Reference_VE = (int16_t)((TPS * MAP[1]) >> 14);
      //correct for TPS flow if used.
      if (TPS_Flow_Cal_On == 1){
	  Reference_VE = ((Reference_VE  * (int16_t)(table_lookup(RPM, TPS, TPS_Flow_Table))) >>14);
      }//if
      //Air temperature correction....I can't figure out how to not make this a divide at the moment
      Reference_VE = (Reference_VE << 14) / (int16_t)IAT;	
  }else{	//(Load_Sense == 5, use MAF 
      gram_flow = ((gram_STP_Air_Per_cc *(uint32_t) Displacement)  >> 12); //convert displacement in cc to g and convert bin 24 to bin 12
      gram_flow = gram_flow * (RPM / 2)>>12; // get g/min and convert bin12 to  bin 0 
      Reference_VE = (int16_t)(((MAF[0] * 60)<<4) / gram_flow) ; //get %VE and convert back to bin 12

  }//if   	    		 
}//Get_Reference_VE


