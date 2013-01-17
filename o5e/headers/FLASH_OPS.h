/**
 * @file   <filename.h>
 * @author <author>
 * @brief  <one liner description>
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __flash_ops_h
#define   __flash_ops_h

#ifdef __cplusplus
extern "C"
{
#endif

/* missing guard, so I added it until I can figure out what's going on here */

#define BLK1B_BASE 0x00008000
#define BLK2A_BASE 0x00010000

/*This function Initializes, Erases and Programs the MPC5xxx FLASH Memory */

void *Flash_Program(uint8_t block, long long *source, uint32_t destination);
void Flash_Erase(uint8_t block);
int32_t Flash_Ready(void);
void Flash_Finish(uint8_t block);

#ifdef __cplusplus
}
#endif

#endif // __flash_ops_h