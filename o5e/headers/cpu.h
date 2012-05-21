
// Include header files that are CPU specific

#ifndef CPU_H
#define CPU_H

#include "config.h"

#ifdef MPC5634
#include "mpc563xm.h"
#endif

#ifdef MPC5554
#include "mpc5554.h"
#endif

#ifdef MPC5602
#include "MPC5602P.h"
#endif

#endif
