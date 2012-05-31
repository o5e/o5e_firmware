
// This file should be included first in every C source file
// It defines various user level CPU and system variables
// Lower level defines and pin assignments should be in system.h
// Users should change this file to match their hardware and compiler

#ifndef CONFIG_H
#define CONFIG_H

// pick one cpu
//#define MPC5554	/* faster, dual eTPU cpu*/
//#define MPC5566
//#define MPC5674
#define MPC5634 	/* smaller cpu */

// pick one board I/O setup
#define TRK_MPC5634
//#define PHYCORE
//#define FREESCALE
//#define NGC4

// custom options
// #define JZ

// pick one 
#define CODEWARRIOR
#undef __PEGNU__
#undef __GNU__

#endif
