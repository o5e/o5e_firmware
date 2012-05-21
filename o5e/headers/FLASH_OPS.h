
#include "cpu.h"

#define BLOCK0 0
#define BLOCK1 1

#ifdef MPC5554
#define M0_BASE 0x00040000
#define H0_BASE 0x00080000
#endif

#ifdef MPC5634
//#define M0_BASE 0x00080000
//#define H0_BASE 0x00100000
#define M0_BASE 0x000A0000
#define H0_BASE 0x00120000
#endif

/*This function Initializes, Erases and Programs the MPC5xxx FLASH Memory */

void init_FLASH(void);

void Erase_H0(void);
void Erase_M0(void);

void *Program_M0(long long *source, uint32_t destination);
void *Program_H0(long long *source, uint32_t destination);

void *Flash_Program(uint_fast8_t block, long long *source, uint32_t destination);
void Flash_Erase(uint_fast8_t block);
int32_t Flash_Ready(void);
void Flash_Finish(uint8_t block);
void Erase_Finish(uint8_t block);
