
// Functions that don't deserve a separate file

void Init_Watchdog(void);
void Feed_Watchdog(void);
int32_t init_eTPU(void);
void init_PWM1(uint32_t freq);
void Init_Tach(void);
void update_PWM1(uint32_t duty_cycle);
void reset_cpu(void);

// Declare tasks
extern void tuner_task(void);
extern void LED_task(void);
extern void O2_Task(void);
extern void Slow_Vars_Task(void);
extern void Eng_Pos_Status_BLINK_Task(void);  // for testing on 5634
extern void Fuel_Pump_Task(void);
extern void Engine10_Task(void);
extern void Enrichment_Task(void);

