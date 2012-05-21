
#include "cpu.h"
#include <stdint.h>

#define eSCI_A_DR  0xFFFB0006   // eSCI Data Reg-16 Bits     
#define eSCI_B_DR  0xFFFB4006

// fast DMA versions 
void init_eSCI_DMA(void);
uint_fast16_t write_serial(const uint8_t *bytes, const uint_fast16_t count);
uint_fast16_t read_serial(uint8_t *bytes, const uint_fast16_t max_bytes);

#define write_serial_busy()  (EDMA.TCD[18].DONE != 1)     // a macro for speed reasons

// slow versions
uint_fast16_t send_serial_A(uint8_t *bytes, uint_fast16_t count);
uint_fast16_t send_serial_B(uint8_t *bytes, uint_fast16_t count);
uint_fast16_t get_serial_A(uint8_t *bytes, uint_fast16_t count);
uint_fast16_t get_serial_B(uint8_t *bytes, uint_fast16_t count);

// compatible with CW printfc
int32_t ReadUARTN(void* bytes, unsigned long limit);
int32_t WriteUARTN(const void* bytes, unsigned long length);
int32_t InitializeUART(int32_t baudRate);
