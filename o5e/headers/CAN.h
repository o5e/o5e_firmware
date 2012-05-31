
void init_CAN (void);
uint_fast16_t write_CAN (const uint8_t * const bytes, const uint_fast16_t count);
uint_fast16_t read_CAN (uint8_t * bytes, const uint_fast16_t max_bytes);
void CAN_task (void);
inline static void receiveMsg ();
inline static void TransmitMsg ();
void initCAN_A(void);


