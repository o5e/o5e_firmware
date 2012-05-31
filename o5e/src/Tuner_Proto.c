/********************************************************************

  @file   Tuner_Proto.c
  @author Jon Zeeff (jon@zeeff.com)
  @date   September, 2011
  @brief  Program to partially implement the MegaTune serial protocol for sending and receiving engine control parameters.
  @warning Lots of commands not implemented yet
  @copyright MIT License
 
  Written as part of the Open5xxxECU project

********************************************************************** */

/*
Copyright (c) 2011 Jon Zeeff
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "config.h"
#include "system.h"
#include <stdint.h>
#include "OS.h"
#include "main.h"
#include "eSCI_DMA.h"
#include "FLASH_OPS.h"
#include "variables.h"
#include "crc.h"
#include <string.h>
#include <stdio.h>
#include "Table_Lookup_JZ.h"
#include "etpu_toothgen.h"
#include "eQADC_OPS.h"
#define EXTERN
#include "Tuner_Proto.h"

// define this if the tuner .ini specifies crc packets
#define CRC

#ifdef CRC
#   define PAYLOAD_OFFSET 2
#else
#   define PAYLOAD_OFSET 0
#endif

#define PAGE_OFFSET 1           // page #s start with 1

int32_t make_packet(uint8_t code, const void *buf, uint16_t size);
int16_t check_crc(uint8_t * packet);

uint32_t Crc32_ComputeBuf(uint32_t inCrc32, const void *buf, uint32_t bufLen);
void tuner_task(void);
int32_t process_packet(const uint8_t * buf, uint16_t size);
void _start(void);

/*  Protocol notes:

<command> <canid> <page> <16bit offset> <16bit size> <data....>

Send (to ECU):
<16bit payload size> <existing command> <CRC32>
(Size is total size of packet excluding wrapper)

Receive (to tuner):
<16bit payload size> <8 bit type> <optional data> <CRC32>
payload size includes the type+data but not CRC
CRC includes type+data but not size

*/

// response codes

#define OK 0x00
#define realtime_data 0x01
#define page_data 0x02
#define config_error 0x03
#define burn_ok 0x04
#define page10_ok 0x05
#define CAN_data 0x06
// Errors
#define underrun 0x80
#define over_run 0x81
#define CRC_failure 0x82
#define unrecognized_command 0x83
#define out_of_range 0x84
#define busy 0x85
#define flash_locked 0x86
#define sequence_failure_1 0x87
#define sequence_failure_2 0x88
#define CAN_queue_full 0x89
#define CAN_timeout 0x8a
#define CAN_failure 0x8b

/*

Example with crc:
Time: 3:29.052: SENT, 7 bytes
x00 x01 x41 xD3 xD9 x9E x8B                                           ..A....
Time: 3:29.073: Received, 3 bytes
x01 x7D x01                                                           .}.
Time: 3:29.277: Received, 384 bytes
x04 x70 x06 x89 x06 x89 x09 xCB x01 x43 x00 x01 x93 x93 x01 x01       .p.......C......

*/

/**
 * @name   tuner_task
 * @brief  processes serial commands from tuner
 * @note   We would like this to be more modular in that a tuner task should know nothing about flash organization. 
 * @note   Unfortunately, use of the OS requires that flash programming be done in this routine.
 *
 */

// this buffer is used to receive and send packets

static uint8_t tmp_buf[SERIAL_BUFFER_SIZE];

void tuner_task(void)
{
    static int_fast16_t count;  // number of bytes received
    static uint_fast16_t i;
    static uint_fast16_t page;
    static uint_fast16_t offset;
    static uint_fast16_t length;
    static uint32_t crc;

    task_open();                // standard OS entry - required on all tasks

    for (;;) {
        task_wait(1);           // let other tasks run
        count = 0;

        // wait until outbound serial port is not busy
        while (write_serial_busy())
            task_wait(111);

        for (;;) {              // loop until we get a packet (delimited by no activity)
            i = read_serial(tmp_buf + count, (uint16_t)(sizeof(tmp_buf) - count));  // get block of bytes from serial port

            if (i == 0 && count > 0)    // we received nothing in the last n msec - process the packet
                break;

            count += i;         // keep receiving
            task_wait(99);
        }                       // for

        // process received packet
        // don't use switch because we use the OS

        // TODO - change .ini file to make these commands full packets, not single characters
        // check for requests without packet wrapper - first byte is command

        if (tmp_buf[0] == 'S') {        // first byte is command
            write_serial((const void *)version, sizeof(version));
            continue;
        }

        if (tmp_buf[0] == 'Q') {        // signature
            write_serial((const void *)signature, sizeof(signature));
            continue;
        }

        // for debugging flash burns
        if (tmp_buf[0] == 'B') {        // test flash burn - assume already erased
            Flash_Program(0, (uint64_t *)"BCDE", 0);   // page 0
            while (!Flash_Ready()) ;
            Flash_Finish(0);

            Flash_Program(1, (uint64_t *)"CDEF", 0);   // page 1
            while (!Flash_Ready()) ;
            Flash_Finish(1);

            write_serial("Burn done", 9);
            continue;
        }

        if (tmp_buf[0] == 'T') {        //  debug - dump a table
            uint8_t *ptr = (uint8_t *) MAP_2_Table;
            uint8_t string[10];
            uint8_t buf[2000];

            buf[0] = '\0';
            for (i = 0; i < 168; ++i) {
                sprintf(string, "0x%x,", *ptr++);
                strcat(buf, string);
            }
            write_serial((const void *)buf, (uint_fast16_t) strlen(buf));
            continue;
        }

        if (tmp_buf[0] == 'D') {        //  dump some variables (debug - ASCII format)
            uint8_t string[100];
            sprintf(string, "N_Cyl = %d\n", (uint32_t) N_Cyl);
            write_serial((const void *)string, (uint_fast16_t) strlen(string));
            task_wait(500);
            sprintf(string, "Injection Time = %d\n", (uint32_t) Injection_Time);
            write_serial((const void *)string, (uint_fast16_t) strlen(string));
            task_wait(500);
            sprintf(string, "CLT(4) = %d bin 14\n", table_lookup_jz(4 << 12, 0, CLT_Table));    // 4V bin 12
            write_serial((const void *)string, (uint_fast16_t) strlen(string));
            task_wait(500);
            if (Error_String[0])
                write_serial((const void *)Error_String, (uint_fast16_t) strlen(Error_String));
            task_wait(500);
            continue;
        }
#if 0
case 'W':                      // debug - write a byte to arbitrary memory location  - W 0xffffffff 0xff
        {
            uint8_t *address;
            uint8_t *ptr;
            uint8_t value;
            uint8_t string[50];
            address = (uint8_t *) atoi(tmp_buf + 1);
            ptr = strchr(tmp_buf + 2, ' ');
            if (!ptr)
                break;
            value = (uint8_t) atoi(ptr);
            *address = value;
            sprintf(string, "Address %x set to %x\r\n", (uint32_t) address, value);
            write_serial((const void *)string, (uint_fast16_t) strlen(tmp_buf));
            break;
        }
case 'R':                      // debug - read arbitrary memory location - R 0xffffffff
        {
            uint8_t *address;
            uint8_t string[100];
            address = (uint8_t *) atoi(tmp_buf + 1);
            sprintf(string, "Address %x = %x\r\n", (uint32_t) address, *address);
            write_serial((const void *)string, (uint_fast16_t) strlen(tmp_buf));
            break;
        }
#endif

        // below processes commands in packets - 3rd byte is command
        // burn command

        if (tmp_buf[2] == 'b') {
#ifdef CRC
            if ((count = check_crc(tmp_buf)) == -1)      // check crc on packet
                break;
#endif
            if (count != 3)     // sanity check
                break;

            // find page #
            page = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 1) - PAGE_OFFSET;
            if (page >= nPages)
                break;

            // optimization: find an unused page within the current block and burn the new page to that - much faster, less wear
            // a burn to page 0 causes a complete block burn so things look cleaner
            uint32_t empty_page;
            if (page > 0 && (empty_page = Find_Empty_Page(Flash_Block))) {

               uint8_t *location = Flash_Addr[Flash_Block] + BLOCK_HEADER_SIZE + (empty_page * Max_Page_Size);   // convert from page # to address 

               if (Burn_Page(Flash_Block,page,(uint_fast16_t)empty_page)) {
                    Page_Ptr[page] = location;          // variables are now moved
                    Ram_Page_Buffer_Page = -1;          // mark as unused
                    make_packet(burn_ok, "", 0);
                    continue;           // done
               } // if

            } // if

            // ignore the page # and burn everything

            // select unused flash block (ping pongs 0 or 1)
            static uint8_t new_flash_block;
            new_flash_block = (Flash_Block == 0) ? 1 : 0;       // select one not in use

            // erase new flash block 
            Flash_Erase(new_flash_block);
            while (!Flash_Ready()) ;
            Erase_Finish(new_flash_block);
            if (*Flash_Addr[new_flash_block] != 0xff) { // check for success
                make_packet(sequence_failure_1, "", 0);
                continue;
            }

            static uint32_t flash_index;                // index into block, not a pointer
            static uint8_t *ptr;

            // write & burn each page to new flash, 8 bytes at a time. 
            for (i = 0; i < nPages; ++i) {

                flash_index = BLOCK_HEADER_SIZE + (uint32_t) (i * Max_Page_Size);    // skip over header
                ptr = (uint8_t *)(Page_Ptr[i]);         // current values, usually in old flash, could be ram

                // write and program 8 bytes at a time 
                for (count = 0; count < pageSize[i]; count += 8) {
                    static uint64_t tmp;               // 8 bytes, properly aligned in ram
                    tmp = *(uint64_t *)ptr;

                    Flash_Program(new_flash_block, &tmp, flash_index);  // setup + copy
                    while (!Flash_Ready())              // wait till ready
                        ;
                    Flash_Finish(new_flash_block);      // terminate 

                    // check it by reading it back
                    if (tmp != *(uint64_t *)(Flash_Addr[new_flash_block] + flash_index)) {
                        uint8_t string[100];
                        sprintf(string, "BAD BURN @%d:%x\n", new_flash_block, flash_index);
                        write_serial((const void *)string, (uint_fast16_t) strlen(string));
                        i = 9999;
                        break;
                    }

                    ptr += 8;   // inc byte pointer to next 8 byte word
                    flash_index += 8;
                }               // for

                task_wait(1);   // let other tasks run

            }                   // for

            // done burning pages

            // write a 8 byte signature at the beginning of this flash block
            {
                static struct Flash_Header header;

                // fill in first 8 bytes of header
                memcpy(header.Cookie, "ABCD", 4);
                header.Burn_Count = ++Burn_Count;
                header.n_Pages = nPages;
                header.Last_Page_Burned = (uint8_t) page;
                // burn first portion of header
                Flash_Program(new_flash_block, (uint64_t *)&header, 0);
                while (!Flash_Ready()) ;                        // wait till burn is done
                Flash_Finish(new_flash_block);
                // let first nPages directory entries be defaults (all 0xff)
                // check for success 
                if (*Flash_Addr[new_flash_block] != 'A') {     
                    make_packet(sequence_failure_2, "", 0);
                    continue;
                }
            }

            // update all Page_Ptrs to point to new, freshly written flash
            Set_Page_Locations(new_flash_block);        // update pointers to new flash

            // we are now running with all variables in the new flash
            make_packet(burn_ok, "", 0);

            continue;
        }
        // check for requests that include a packet wrapper 

        // 'A' is send all output variables
        if (tmp_buf[2] == 'A') {
            if ((count = check_crc(tmp_buf)) == -1)      // check crc on packet
                continue;
            make_packet(OK, (const void *)&Output_Channels, Output_Channels_Size);
            continue;
        }

        // transfer (pass through) packet to CAN bus
        if (tmp_buf[2] == 'C') {
            if ((count = check_crc(tmp_buf)) == -1)      // check crc on packet
                continue;
            // TODO
            //write_CAN(tmp_buf+PAYLOAD_OFFSET,count);
            make_packet(OK, "", 0);
            continue;
        }

        // read page from flash
        if (tmp_buf[2] == 'r') {
            if ((count = check_crc(tmp_buf)) == -1)      // check crc on packet
                continue;
            if (count != 7)     // sanity check
                continue;

            // find page #
            page = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 1) - PAGE_OFFSET;
            if (page >= nPages)
                continue;
            // find offset
            offset = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 3);
            // find length
            length = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 5);

            make_packet(OK, (void *)(Page_Ptr[page] + offset), length);
            continue;
        }

        // write page to flash
        if (tmp_buf[2] == 'w') {
            // Example: SENT, 15 bytes
            // x00 x09 x77 x00 x05 x00 x00 x00 x02 x00 x18 x30 x1B xD9 x2D 
            // Example: without CRC
            // x77 x00 x05 x00 x00 x00 x02 x00 x18

            if ((count = check_crc(tmp_buf)) == -1)      // check crc on packet
                continue;

            if (count < 7 || count > Max_Page_Size)     // sanity check
                continue;

            // find page #
            page = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 1) - PAGE_OFFSET;
            if (page >= nPages)
                continue;
            // find offset
            offset = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 3);
            // find length
            length = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 5);

            if (offset + length > Max_Page_Size)
                continue;       // too big

            if (Ram_Page_Buffer_Page != -1 && Ram_Page_Buffer_Page != page)     // some other page is using the ram buffer
                continue;

            // if first write to this page, copy previous data from flash to ram
            if (Page_Ptr[page] != Ram_Page_Buffer) {

                // copy  
                memcpy(Ram_Page_Buffer, (void *)(Page_Ptr[page]), pageSize[page]);

                // move ptr from flash to ram - puts new data into use
                Page_Ptr[page] = Ram_Page_Buffer;
                Ram_Page_Buffer_Page = (int8_t)page;    // mark as in use
            }
            // copy new data from tuner to ram page buffer
            memcpy(Ram_Page_Buffer + offset, tmp_buf + PAYLOAD_OFFSET + 7, length);

            // send response
            make_packet(OK, "", 0);
            continue;
        }

        // send crc32 of flash page
        if (tmp_buf[2] == 'k') {        
            if ((count = check_crc(tmp_buf)) == -1)      // check crc on packet
                continue;
            if (count != 7) {   // sanity check
                write_serial(tmp_buf, 7);       // helps debug
                continue;
            }
            // find page #
            page = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 1) - PAGE_OFFSET;
            if (page >= nPages) {
                write_serial("BAD PAGE", 8);    // helps debug
                continue;
            }
            // find offset - ignored
            offset = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 3);
            if (offset >= pageSize[page]) {
                write_serial("BAD OFFSET", 10);
                continue;
            }
            // find length - ignored 
            length = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 5);
            if (offset + length >= pageSize[page]) {
                write_serial("BAD LENGTH", 10);
                continue;
            }
            // find requested crc
            crc = Crc32_ComputeBuf(0, (void *)(Page_Ptr[page]), pageSize[page]);

            make_packet(OK, (const void *)&crc, sizeof(crc));
            continue;
        }

        // no match to command
        make_packet(unrecognized_command, "", 0);

    }                           // for ever

    task_close();
}                               // tuner_task()

// Add byte count to the beginning of a packet and a crc at the end.  Then send the packet.

int32_t make_packet(const uint8_t code, const void *buf, const uint16_t size)
{

// Do we send size and CRC value on response (set in .ini file)?
#ifdef CRC
    *(uint16_t *) tmp_buf = size + 1;   // size - includes code
    *(uint8_t *) (tmp_buf + 2) = code;  // code
    memcpy(tmp_buf + 3, buf, size);     // data
    *(uint32_t *) (tmp_buf + 2 + 1 + size) = Crc32_ComputeBuf((uint32_t) 0, (void *)(tmp_buf + 2), (uint32_t) (size + 1));      // CRC on code + data

    write_serial((const void *)tmp_buf, size + 2 + 1 + 4);      // with size, code and crc
#else
    if (size > 0)
        write_serial(buf, size);        // bare
    else
        write_serial((const void *)&code, 1);   // bare
#endif

    return 0;
}

#ifdef CRC
// return size or -1 if packet is corrupt

// check the crc value in a packet

int16_t check_crc(uint8_t * packet)
{
    static uint32_t crc;
    static int16_t size;

    // get size of data from the packet (doesn't include size or crc)
    size = *(int16_t *) packet;
    if (size > Max_Page_Size || size < 1) {
        make_packet(over_run, "", 0);
        return -1;
    }
    // verify packet crc
    crc = *(uint32_t *) (packet + 2 + size);
    if (crc != Crc32_ComputeBuf(0, packet + 2, size)) {
        make_packet(CRC_failure, "", 0);
        return -1;
    }

    return size;                // size of payload
}
#endif


// search through a flash block for a page that is empty
uint8_t  
Find_Empty_Page(uint8_t block) 
{
uint8_t *location = Flash_Addr[block];                  // base address
uint8_t *end = location + ((128-1) * Max_Page_Size) ;   // blocks are 128K long
uint8_t page = nPages;                                  // skip always used pages

// skip over the block header and the pages that are always used
location +=  BLOCK_HEADER_SIZE +  Max_Page_Size * nPages;

while (location < end) {
      if (Page_Is_Blank(location))
         return page;   // found a usable page

      // try the next page in this block
      location += Max_Page_Size;
      ++page;

}     // while

return 0;   // no pages are usable

} // Find_Empty_Page()

// check if page is completely blank

uint8_t
Page_Is_Blank(uint8_t *ptr)
{
      uint32_t * ptr32 = (uint32_t *) ptr;  // faster to do 4 bytes at a time
      int i;

      for (i = 0; i < Max_Page_Size; i += sizeof(uint32_t)) {
          if (*ptr32  != 0xffffffff)  // 0xff is value of erased flash
             return 0;
          ++ptr32;                  // move to next word
      } // for

      return 1;
}

// Burn a page of flash
// Note, task_wait() cannot be called from task subroutines

uint8_t 
Burn_Page(uint8_t block,  uint_fast16_t page, uint_fast16_t destination_page)
{
int count;
uint8_t *from = (uint8_t *)(Page_Ptr[page]);                                  // from address
uint32_t flash_index = BLOCK_HEADER_SIZE + destination_page * Max_Page_Size;  // to index

                // write and program 8 bytes at a time 
                for (count = 0; count < pageSize[page]; count += 8) {
                    uint64_t tmp;               // 8 bytes, properly aligned in ram
                    tmp = *(uint64_t *)from;

                    Flash_Program(block, &tmp, flash_index);    // setup + copy
                    while (!Flash_Ready())                      // wait till ready
                        ;
                    Flash_Finish(block);        // terminate 

                    // check it by reading it back
                    if (tmp != *(uint64_t *)(Flash_Addr[block] + flash_index)) {
                        return 0;               // failure
                    }

                    from += 8;                  // inc byte pointer to next 8 byte word
                    flash_index += 8;           // inc destination
                }               // for

                // record location of this page in the directory
                uint64_t i = page;              // 8 byte version
                Flash_Program(block, &i, BLOCK_HEADER_DIRECTORY  + (destination_page * sizeof(uint64_t)));  // find offset to correct entry
                while (!Flash_Ready()) ;        // wait till burn is done
                Flash_Finish(block);

                return 1; // success

} // Burn_Page()

