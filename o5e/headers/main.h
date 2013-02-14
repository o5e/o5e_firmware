
#ifndef MAIN_H
#define MAIN_H

// general purpose clocks that all tasks can use
extern uint32_t Degree_Clock;		  	// crank degrees since engine start - won't roll over
void Angle_Clock_Task(void);
#endif