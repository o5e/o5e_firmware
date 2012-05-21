
// This file should be included first in every C source file
// It defines various user level CPU and system variables
// Lower level defines and pin assignments should be in system.h
// Users should change this file to match their hardware and compiler

#ifndef CONFIG_H
#define CONFIG_H

// pick one 
//#define MPC5554	/* open5xxxECU standard CPU */
#define MPC5634 	/* low cost development board */
//#define MPC5602	/* very low cost development board with no eTPU */

// custom options
// #define JZ

// pick one 
#define CODEWARRIOR
#undef __PEGNU__
#undef __GNU__

#endif
