
#ifndef Engine_OPS_H
#define Engine_OPS_H

#ifndef EXTERN
#define EXTERN extern
#endif

extern void Slow_Vars_Task(void);
extern void Fuel_Pump_Task(void);
extern void Set_Spark(void);
extern void Set_Fuel(void);
extern void Engine10_Task(void);

EXTERN uint32_t Pulse_Width;

#endif
#undef EXTERN

