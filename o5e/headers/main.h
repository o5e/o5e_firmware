
#ifndef MAIN_H
#define MAIN_H

#ifndef EXTERN
#define EXTERN extern
#endif

// general purpose clocks that all tasks can use
EXTERN uint32_t msec_clock;    			// anything can use this one tick per msec clock - never rolls over
EXTERN uint32_t Angle_Ticks_Per_Degree;    	// how fast TCR2 increases
EXTERN uint32_t Degree_Clock;		  	// crank degrees since engine start - won't roll over

void system_error(uint16_t code, char *file, uint16_t line, char *note);

#endif
#undef EXTERN
