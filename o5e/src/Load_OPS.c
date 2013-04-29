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




#if __CWCC__
#pragma push
#pragma warn_unusedvar    off
#pragma warn_implicitconv off
#endif
/* the above is inserted until I can figure out how this code works
   and fix it properly */
void Get_Load(void)
{  
        // TODO  - add load sense method selection and calcs. This only works right with 1 bar MAP
        // Load = Get_Load();
        Load = (MAP[1] << 2);   // convert bin 12 to 14 and account for /100Kpa using MAP 2 until angle reading fixed
}

#if __CWCC__
#pragma pop
#endif
