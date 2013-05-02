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
#include "Variable_OPS.h"
#include "Table_Lookup_JZ.h"
#include "Load_OPS.h"

uint32_t gram_flow;
#define Displacement 2000 //test value  dicplacement in cc,
#define gram_STP_Air_Per_cc 21642 //gram STP air/cc - this is a bin24


#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif


/* the above is inserted until I can figure out how this code works
   and fix it properly */
void Get_Load(void)
{  
     
if (Load_Sense <= 3){
    Load = MAP[1];//  using MAP 2 until angle reading fixed on MAP1 (MAP[0])
    // Air temperature correction....I can't figure out how to not make this a divide at the moment
    Load = (Load << 14) / IAT;	
}

else if (Load_Sense == 4){
    Load = ((TPS * MAP[1]) >> 14);
    // Air temperature correction....I can't figure out how to not make this a divide at the moment
    Load = (Load << 14) / IAT;	
}
    
else{	//(Load_Sense == 5, use MAF 
    
    gram_flow = ((gram_STP_Air_Per_cc * Displacement)  >> 12); //convert displacement in cc to g and convert bin 24 to bin 12
    gram_flow = gram_flow * (RPM / 2)>>12; // get g/min and convert bin12 to  bin 0 
    Load = (((MAF[0] * 60)<<4) / gram_flow) ; //get %VE and convert back to bin 12

    }//else   	    		 
}

#if __CWCC__
#pragma pop
#endif
