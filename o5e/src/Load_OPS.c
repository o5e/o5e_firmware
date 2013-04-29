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

int16_t Load_Type;


#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif

void Set_Load_Type(void)
{

}



/* the above is inserted until I can figure out how this code works
   and fix it properly */
void Get_Load(void)
{  
        
if (Load_Sense <= 3)
    Load = (MAP[1] << 2);// convert bin 12 to 14 and account for /100Kpa using MAP 2 until angle reading fixed
else if (Load_Sense == 4)
    Load = ((TPS * MAP[1]) >> 12);
else //(Load_Sense == 5) - MAF 
   Load = (1 << 14);	//set  to 100% until I decide how to handle this.
   //MAF[0] = (int16_t) table_lookup_jz(V_MAF[0], 0, MAF_1_Table);	    		 
}

#if __CWCC__
#pragma pop
#endif
