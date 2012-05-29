/**********************************************************************************************

@file   Tuner_OPS.c 
@author Jon Zeeff 
@date   October, 2011
@brief  Open5xxxECU - handles communications with a tuner
@version 1.0
@copyright MIT License

**********************************************************************************/
/*
Copyright (c) 2011 Jon Zeeff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "config.h"
#include "main.h"
#include <string.h>
#define EXTERN
#include "Tuner_OPS.h"
#include "Packet.h"
#include "Serial.h"
#include "eSCI_DMA.h"
#include "Flash_OPS.h"
#include "OS.h"

static uint8_t tmp_buf[SERIAL_BUFFER_SIZE];	// just to hold it while we process it
static uint8_t stream[MAX_PACKET_SIZE];     // buffer ready for serial output

// this task runs continuously, reads from serial port, interprets packets, sends response

void tuner_task(void)
{
	static int32_t i;
	static int32_t count;
	static int32_t size;
	static uint8_t *stream_ptr;
	
	task_open();		// standard OS entry - required on all tasks

	for (;;) {
		count = read_serial(tmp_buf, sizeof(tmp_buf));	// get block of bytes from serial port
		for (i = 0; i < count; --i) {					// process block
			if ((size = process_serial_byte(tmp_buf[i]) > 0)) {	// if we receieved a full packet/command
				
				//while (flash_busy())                    // flash resource may be needed, make sure it is 
				task_wait(10);						// available before proceeding
				
				size = process_packet(packet, size);	// go do it, response returns in same packet
				if (size > 0) {							// if there is a response to send
					
					size = packetize_output(packet, stream, size);   // convert packet to stream
					stream_ptr = stream;

					while (size > 0) {               	// loop to send it out
						while (write_serial_busy())
						task_wait(4);		// let other tasks run while waiting for Q to empty

						// send as much of response as possible
						static uint_fast16_t bsize;
						bsize = min(size, SERIAL_BUFFER_SIZE);
						write_serial((const uint8_t *) stream_ptr, bsize);	// send response
						stream_ptr += bsize;	// move to remaining bytes
						size -= bsize;			// track remaining
					}

				}	// while
				task_wait(1);	// let other tasks run 
			}	// for
		}
		task_wait(10);	// let other tasks run  - this value is critical to prevent Q overflow
	}			// for ever
	task_close();
}				// tuner_task()

// calc an error prone, slow, overly simple 16 (or 8) bit checksum on some bytes
uint16_t checksum(const uint8_t * addr, const uint_fast16_t count)
{
	register uint_fast16_t sum = 0;
	register uint_fast16_t n;

	n = count >> 3;
	while (n--) {		// unrolled for speed
		sum += *addr++;	// faster and cleaner than Duff's Device
		sum += *addr++;
		sum += *addr++;
		sum += *addr++;
		sum += *addr++;
		sum += *addr++;
		sum += *addr++;
		sum += *addr++;
	}
	n = count & 0x7;
	while (n--)		// remainder - not worth worrying about
	sum += *addr++;

	return sum;
}				// checksum()

#if 0
// example of fixed address
#pragma push
#pragma section code_type ".abs.00010000" code_mode=pc_rel
void func(void)
{
}
#pragma pop
#endif

#if 0
Notes:
'poll' the DONE and PEG 
Flash M0 0x40000
Flash H0 0x80000


// Take all the current tables, wherever they are, and write them to a new flash block

flash_write_task() {

	// select unused new flash block (ping pongs 0 or 1)
	new_flash_block = (flash_block == 0) ? 1 : 0;

	new_flash_addr = ???0x40000 + 0x40000 * flash_block;   // M0 or H0 block
	// verify that new flash block is erased 
	if (*new_flash_addr != 0xffffffff) {
		// erase it
		Flash_Erase(new_flash_block);
		while (!Flash_Ready()) 
		os_wait(100);
	}

	// write each table to new flash(8 byte alignment, attend to other tasks every 8 bytes)
	flash_ptr = new_flash_addr;
	// reserve room for header (magic cookie, table of tables)
	flash_ptr += sizeof(struct ToT_header) + sizeof(struct TableOfTable);


	Flash_Erase(1);
	while (!Flash_Ready()) {};
	Flash_Finish();
	Flash_Program(BLOCK0,array,0);
	
	
	// for each table, write to new flash
	for (i = 0; i < MAX_TABLES; ++i )
	// update table of tables with new flash address
	table->flash_address = flash_ptr;

	count = tablesize(table[i]);
	// find it (in current flash or ram)
	ptr = xx&table[i];

	// write 8 bytes at a time
	while (count > 0) {
		i = min(count,8);
		Flash_Program(ptr,flash_ptr,i);
		count -= i;
		flash_ptr = i;
		while (!Flash_Ready()) 
		task_wait(1);          // run other tasks
		Flash_Finish();
	} // while
} // for

// write new table of tables to new flash
flash_ptr = new_flash_addr + sizeof(struct TofT_header);
while (count > 0) {
	write_flash(table_of_tables,flash_ptr,sizeof(table_of_tables));
	while (!Flash_Ready()) 
	task_wait(1);          // run other tasks
	Flash_Finish();
}

// write magic cookie (4 bytes + 4 byte serial #)
flash_ptr = new_flash_addr;
++TofT_header->serial_number;
Flash_Program(flash_header,xxflash_ptr,sizeof(flash_header));
while (!Flash_Ready()) 
task_wait(1);          // run other tasks
Flash_Finish();

// update working copy of TofT + pointers
// note: now running from tables in new flash
// release ram used(free()) by tables in ram
for (i = 0; i < MAX_TABLES; ++i )
if (in_ram(table[i]) 
		free(table[i]);

		// erase old flash block while running other tasks
		Flash_Erase(old_flash_block);
		while (!Flash_Ready) 
		os_wait(100);
		Flash_Finish();
		
		// exit task
		
		SRAM_START SRAM_END ? ? FLASH1_START ? ? FLASH2_START
#endif
