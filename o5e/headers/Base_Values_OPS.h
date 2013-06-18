#ifndef Base_Values_OPS_H
#define Base_Values_OPS_H

#define Base_Air_Fuel_Ratio 1115       // using 1/14.7 bin 14
#define Gasoline_SG   5642             // cc/g bin 12
#define gram_STP_Air_Per_cc 21642 //gram STP air/cc - this is a bin24

extern float Base_Pulse_Width;
extern float Inverse_Injector_Pressure; //bin 20
extern float Injector_Flow;

void Get_Base_Pulse_Width(void);


#endif
