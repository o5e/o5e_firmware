#ifndef Base_Values_OPS_H
#define Base_Values_OPS_H

#define Base_Air_Fuel_Ratio 1.0f/14.7f       // 
#define Gasoline_SG   1.3774f             // cc/g
#define gram_STP_Air_Per_cc 0.00129f //gram STP air/cc

extern float Base_Pulse_Width;
extern float Inverse_Injector_Pressure; 
extern float Injector_Flow;

void Get_Base_Pulse_Width(void);


#endif
