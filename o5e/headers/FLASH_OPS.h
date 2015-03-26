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

/* Block names like user manual to index into attributes array below */
#define BLOCK0      0
#define BLOCK1A     1
#define BLOCK1B     2
#define BLOCK2A     3
#define BLOCK2B     4
#define BLOCK3      5
#define BLOCK4      6
#define BLOCK5      7
#define BLOCK6      8
#define BLOCK7      9
#define BLOCK8     10


struct flash_attr {
	uint16_t bitno_mlr;
	uint32_t size;
	uint8_t  *addr;
};

extern struct flash_attr flash_attr[];

/*These functions Initialize, Erase and Program the MPC5xxx FLASH Memory */

void flash_program(uint32_t block, long long *source, uint32_t destination);
void flash_erase( uint32_t block );
int32_t Flash_Ready(void);
void Flash_Finish();

#ifdef __cplusplus
}
#endif

#endif // __flash_ops_h