#ifndef Engine_OPS_H
#define Engine_OPS_H


void Slow_Vars_Task(void);
void Fuel_Pump_Task(void);
uint8_t GetEngineState(void);
void Engine_State_UnknownFunc();
void Engine_State_CrankingFunc();
void Engine_State_Cranking_SyncedFunc();
void Engine_State_Running_Over_RevFunc();
void Engine_State_Running_Over_Rev_WarmupFunc();
void Engine_State_Running_Wheel_SlipFunc();
void Engine_State_Running_Wheel_Slip_WarmupFunc();
void Engine_State_Running_AcceleratingFunc();
void Engine_State_Running_Accelerating_WarmupFunc();
void Engine_State_Running_DeceleratingFunc();
void Engine_State_Running_Decelerating_WarmupFunc();
void Engine_State_Running_NormalFunc();
void Engine_State_Running_Normal_WarmupFunc();
void Engine_State_Running_IdleFunc();
void Engine_State_Running_Idle_WarmupFunc();
void Engine_State_Running_Idle_SettlingFunc();
void Engine_State_Running_Idle_Settling_WarmupFunc();

void Engine10_Task(void);
void Eng_Pos_Status_BLINK_Task(void);
void Cam_Pulse_Task(void);
void Test_RPM_Task(void);

#endif
