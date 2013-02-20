/********************************************************************

  @file   Tuner.c
  @author Jon Zeeff (jon@zeeff.com)
  @date   September, 2011
  @brief  Program to implement the Tuner Studio serial protocol
          for sending and receiving engine control parameters
  @copyright 2011, 2012 Jon Zeeff

***********************************************************************/

/* Copyright (c) 2011,2012 Jon Zeeff
   Copyright 2012, Sean Stasiak <sstasiak at gmail dot com> */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "mpc563xm.h"
#include "config.h"
#include "cocoos.h"
#include "err.h"
#include "eSCI_DMA.h"
#include "FLASH_OPS.h"
#include "variables.h"
#include "Tuner.h"

#define PAYLOAD_OFFSET 2        // accounts for packet with crc and size
#define PAGE_OFFSET 1           // page #s start with 1

#define write_serial_busy()  (EDMA.TCD[18].DONE != 1)     // a macro for speed reasons

// Ram buffer to store a single page before writing to flash
static uint8_t Ram_Page_Buffer[MAX_PAGE_SIZE];

/*  Protocol notes:

<command> <canid> <page> <16bit offset> <16bit size> <data....>

Send (to ECU):
<16bit payload size> <existing command> <CRC32>
(Size is total size of packet excluding wrapper)

Receive (to tuner):
<16bit payload size> <8 bit type> <optional data> <CRC32>
payload size includes the type+data but not CRC
CRC includes type+data but not size

Special pages:
0xf0    ; tooth logger
0xf1    ; trigger logger
0xf2    ; composite tooth logger
0xf3    ; composite tooth logger loop

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
#define auth_failure 0x8c


/*

Example with crc:
Time: 3:29.052: SENT, 7 bytes
x00 x01 x41 xD3 xD9 x9E x8B                                           ..A....
Time: 3:29.073: Received, 3 bytes
x01 x7D x01                                                           .}.
Time: 3:29.277: Received, 384 bytes
x04 x70 x06 x89 x06 x89 x09 xCB x01 x43 x00 x01 x93 x93 x01 x01       .p.......C......

*/

void __start(void);
static uint32_t Crc32_ComputeBuf(uint32_t inCrc32, const void *buf, uint32_t bufLen);
static uint16_t write_tuner(const uint8_t *bytes, const uint16_t count);
static int16_t check_crc(uint8_t * packet);
static int32_t make_packet(uint8_t code, const void *buf, uint16_t size);
static uint8_t Page_Is_Blank(uint8_t *ptr);

/**
 * @name   Tuner_Task
 * @brief  processes serial commands from tuner
 * @note   We would like this to be more modular in that a tuner task should know nothing about flash organization.
 * @note   Unfortunately, use of the OS requires that flash programming be done in this routine.
 *
 */

static uint8_t tmp_buf[SERIAL_BUFFER_SIZE];  // this buffer is used to receive and send packets

void Tuner_Task(void)
{
    static uint8_t password_received = 0;     // have we received a valid password to allow flash reading
    static uint32_t Password;                 // move to flash
    static int16_t count;  // number of bytes received
    static uint16_t i;
    static uint16_t page;
    static uint16_t offset;
    static uint16_t length;
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

        // check for requests without packet wrapper (ie, manually entered from terminal) - first byte is command

        if (tmp_buf[0] == 'S' && count == 1) {        // version
            write_tuner((const void *)VERSION, sizeof(VERSION));
            continue;
        }

        if (tmp_buf[0] == 'T' && count == 1) {        //  reboot cpu
            __start();  /**< this is NOT a reboot, this is a restart */
            continue;
        }

        if (tmp_buf[0] == 'F' && count == 1) {        // protocol interrogation
            write_tuner((const void *)PROTOCOL, sizeof(PROTOCOL));
            continue;
        }

        if (tmp_buf[0] == 'Q' && count == 1) {        // signature
            write_tuner((const void *)SIGNATURE, sizeof(SIGNATURE));
            continue;
        }

        // below processes commands in packets - 3rd byte is command

        if (tmp_buf[2] == 'Q') {                        // signature
            if ((count = check_crc(tmp_buf)) == -1) {   // check crc on packet
                err_push( CODE_OLDJUNK_F9 );            // using same error code as below...  not sure how our scheme is. 
                continue;
            }

            static uint8_t signature[20] = SIGNATURE;
            make_packet(OK, (const void *)&signature, sizeof(signature));
            continue;
        }


        if (tmp_buf[2] == 'R') {                        // send random number
            if ((count = check_crc(tmp_buf)) == -1) {   // check crc on packet
                err_push( CODE_OLDJUNK_F9 );
                continue;
            }

            static uint64_t random=1234;
            make_packet(OK, (const void *)&random, sizeof(random));
            continue;
        }

        if (tmp_buf[2] == 'S') {                        // receive authorization
            if ((count = check_crc(tmp_buf)) == -1) {   // check crc on packet
                err_push( CODE_OLDJUNK_F8 );
                continue;
            }

			static uint8_t version[59] = VERSION;
			make_packet(OK, (const void *)version, sizeof(version));
          continue;
        }

        // burn command
        if (tmp_buf[2] == 'b') {                        // burn
            if ((count = check_crc(tmp_buf)) == -1) {   // check crc on packet
                err_push( CODE_OLDJUNK_F7 );
                continue;
            }

            if (count != 3)  {   // sanity check
                err_push( CODE_OLDJUNK_F6 );
                continue;
            }

            // ignore the page # and burn everything to the other block
            // select unused flash block (ping pongs 0 or 1)
            static uint8_t new_flash_block;
            new_flash_block = Flash_Block ^ 1;       // toggle to one not in use

            // erase new flash block
            if (!Page_Is_Blank(Flash_Addr[new_flash_block])) {  // if not already erased
              asm("wrteei 0");
               Flash_Erase(new_flash_block);
               while (!Flash_Ready()) { };
               Flash_Finish(new_flash_block);
              asm("wrteei 1");
            }

            // check erase
            if (!Page_Is_Blank(Flash_Addr[new_flash_block])) {
                err_push( CODE_OLDJUNK_F4 );
                make_packet(sequence_failure_1, "", 0);
                continue;
            }

            static uint32_t flash_index;                // index into block, not a pointer
            static uint8_t *ptr;

            // write & burn each page to new flash, 8 bytes at a time.
            for (i = 0; i < NPAGES; ++i) {

                flash_index = BLOCK_HEADER_SIZE + (uint32_t) (i * MAX_PAGE_SIZE);    // skip over header
                ptr = (uint8_t *)(Page_Ptr[i]);         // current values, usually in old flash, could be ram

                // write and program 8 bytes at a time
                for (count = 0; count < pageSize[i]; count += 8) {
                    static uint64_t tmp;               // 8 bytes, properly aligned in ram
                    tmp = *(uint64_t *)ptr;

                    asm("wrteei 0");
                    Flash_Program(new_flash_block, &tmp, flash_index);  // setup + copy
                    while (!Flash_Ready()) { };                         // wait till ready
                    Flash_Finish(new_flash_block);                      // terminate
                    asm("wrteei 1");

                    // check it by reading it back
                    if (tmp != *(uint64_t *)(Flash_Addr[new_flash_block] + flash_index)) {
                        err_push( CODE_OLDJUNK_F3 );
                        i = 9999;  // abort out loop
                        break;
                    }

                    ptr += 8;   // inc byte pointer to next 8 byte word
                    flash_index += 8;
                }               // for

                // task_wait(1);   // let other tasks run

                // check again - compare full page
                flash_index = BLOCK_HEADER_SIZE + (uint32_t) (i * MAX_PAGE_SIZE);    // skip over header
                ptr = (uint8_t *)(Page_Ptr[i]);         // current values, usually in old flash, could be ram
                if (memcmp(ptr,Flash_Addr[new_flash_block] + flash_index,pageSize[i])) {
                   err_push( CODE_OLDJUNK_F2 );
                }

            }                   // for

            // done burning pages
            // write a 8 byte SIGNATURE at the beginning of this flash block
            {
                static struct Flash_Header header;

                // fill in first 8 bytes of header
                memcpy(header.Cookie, "ABCD", 4);
                header.Burn_Count = ++Burn_Count;
                // burn first portion of header
                asm("wrteei 0");
                Flash_Program(new_flash_block, (uint64_t *)&header, 0);
                while (!Flash_Ready()) { };                // wait till burn is done
                Flash_Finish(new_flash_block);
                asm("wrteei 1");
                // check for success
                if (*Flash_Addr[new_flash_block] != 'A') {
                    err_push( CODE_OLDJUNK_F1 );
                    make_packet(sequence_failure_2, "", 0);
                    continue;
                }
            }

            // update all Page_Ptrs to point to new, freshly written flash
            Set_Page_Locations(new_flash_block);                // update pointers to new flash

            // erase old block
            new_flash_block ^= 1;               // the new new one
            asm("wrteei 0");
            Flash_Erase(new_flash_block);
            while (!Flash_Ready()) { };
            Flash_Finish(new_flash_block);
            asm("wrteei 1");

            // we are now running with all variables in the new flash
            make_packet(burn_ok, "", 0);

            // make sure response gets out
            while (write_serial_busy()) {};
            continue;
        }

        // check for requests that include a packet wrapper

        // 'A' is send all output variables
        if (tmp_buf[2] == 'A') {
            if ((count = check_crc(tmp_buf)) == -1) {      // check crc on packet
                err_push( CODE_OLDJUNK_F0 );
                continue;
            }
            make_packet(OK, (const void *)&Output_Channels, OUTPUT_CHANNELS_SIZE);
            continue;
        }


        // read page from flash
        if (tmp_buf[2] == 'r') {
            if ((count = check_crc(tmp_buf)) == -1)      // check crc on packet
                continue;
            if (count != 7)     // sanity check
                continue;

            // reading flash may be password protected
            if (Password != 0xffffffff && Password != 0 && !password_received) {
                err_push( CODE_OLDJUNK_EF );
                continue;
            }

            // find page #
            page = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 1) - PAGE_OFFSET;
            if (page >= NPAGES)
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

            if ((count = check_crc(tmp_buf)) == -1) {     // check crc on packet
                err_push( CODE_OLDJUNK_EE );
                continue;
            }

            if (count < 7 || count > MAX_PAGE_SIZE) {     // sanity check
                err_push( CODE_OLDJUNK_ED );
                continue;
            }

            // find page #
            page = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 1) - PAGE_OFFSET;
            if (page >= NPAGES) {
                err_push( CODE_OLDJUNK_EC );
                continue;
            }

            // find offset
            offset = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 3);

            // find length
            length = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 5);

            if (offset + length > MAX_PAGE_SIZE) {
                err_push( CODE_OLDJUNK_EB );
                continue;       // too big
            }

            if (Ram_Page_Buffer_Page != -1 && Ram_Page_Buffer_Page != page)  {    // some other page is using the ram buffer
                err_push( CODE_OLDJUNK_EA );
                continue;
            }

            // if first write to this page, copy previous data from flash to ram
            if (Page_Ptr[page] != Ram_Page_Buffer) {
                // copy
                memcpy(Ram_Page_Buffer, (void *)(Page_Ptr[page]), MAX_PAGE_SIZE);

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

            if (count != 7) {                           // sanity check
                err_push( CODE_OLDJUNK_E9 );
                continue;
            }
            // find page #
            page = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 1) - PAGE_OFFSET;
            if (page >= NPAGES) {
                err_push( CODE_OLDJUNK_E8 );
                continue;
            }
            // find offset - ignored
            offset = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 3);
            if (offset >= pageSize[page]) {
                err_push( CODE_OLDJUNK_E7 );
                continue;
            }
            // find length - ignored
            length = *(uint16_t *) (tmp_buf + PAYLOAD_OFFSET + 5);
            if (offset + length >= pageSize[page]) {
                err_push( CODE_OLDJUNK_E6 );
                continue;
            }
            // find requested crc
            crc = Crc32_ComputeBuf(0, (void *)(Page_Ptr[page]), pageSize[page]);

            make_packet(OK, (const void *)&crc, sizeof(crc));
            continue;
        }

        // receive 32 bit password to allow flash reading
        if (tmp_buf[2] == 'p') {
            if ((count = check_crc(tmp_buf)) == -1)     // check crc on packet
                continue;

            if (count != 8) {                           // sanity check
                err_push( CODE_OLDJUNK_E5 );
                continue;
            }

            uint32_t pass;
            memcpy(&pass,tmp_buf + PAYLOAD_OFFSET + 1,sizeof(pass));

            if (!Password || pass == Password)  {       // success
               password_received = 1;
               make_packet(OK, (const void *)&crc, sizeof(crc));
            } else
               __start();   // reboot /**< this is NOT a reboot, this is a restart */

            continue;
        }

        // no match to command
        make_packet(unrecognized_command, "", 0);

    }                           // for ever

    task_close();
}                               // Tuner_Task()

// write to serial or CAN when it is ready
static uint16_t write_tuner(const uint8_t *bytes, const uint16_t count)
{
      while (write_serial_busy()) {};   // serial uses DMA, nothing to do but wait
      return write_serial(bytes, count);
}

// Add byte count to the beginning of a packet and a crc at the end.  Then send the packet.

static int32_t make_packet(const uint8_t code, const void *buf, const uint16_t size)
{
    *(uint16_t *) tmp_buf = size + 1;   // size - includes code
    *(uint8_t *) (tmp_buf + 2) = code;  // code
    memcpy(tmp_buf + 3, buf, size);     // data
    // CRC on code + data
    *(uint32_t *) (tmp_buf + 2 + 1 + size) = Crc32_ComputeBuf((uint32_t) 0, (void *)(tmp_buf + 2), (uint32_t) (size + 1));

    write_tuner((const void *)tmp_buf, size + 2 + 1 + 4);      // with size, code and crc

    return 0;
}

// return size or -1 if packet is corrupt

// check the crc value in a packet

static int16_t check_crc(uint8_t * packet)
{
    static uint32_t crc;
    static int16_t size;

    // get size of data from the packet (doesn't include size or crc)
    size = *(int16_t *) packet;
    if (size > MAX_PAGE_SIZE || size < 1) {
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

// check if page is completely blank
static uint8_t
Page_Is_Blank(uint8_t *ptr)
{
      uint32_t * ptr32 = (uint32_t *) ptr;  // faster to do 4 bytes at a time
      int i;

      for (i = 0; i < MAX_PAGE_SIZE; i += sizeof(uint32_t)) {
          if (*ptr32  != 0xffffffff)  // 0xff is value of erased flash
             return 0;
          ++ptr32;                  // move to next word
      } // for

      return 1;
}

/*----------------------------------------------------------------------------*\
 *  NAME:
 *     Crc32_ComputeBuf() - computes the CRC-32 value of a memory buffer
 *  DESCRIPTION:
 *     Computes or accumulates the CRC-32 value for a memory buffer.
 *     The 'inCrc32' gives a previously accumulated CRC-32 value to allow
 *     a CRC to be generated for multiple sequential buffer-fuls of data.
 *     The 'inCrc32' for the first buffer must be zero.
 *  ARGUMENTS:
 *     inCrc32 - accumulated CRC-32 value, must be 0 on first call
 *     buf     - buffer to compute CRC-32 value for
 *     bufLen  - number of bytes in buffer
 *  RETURNS:
 *     crc32 - computed CRC-32 value
 *  ERRORS:
 *     (no errors are possible)
\*----------------------------------------------------------------------------*/

static uint32_t Crc32_ComputeBuf(uint32_t inCrc32, const void *buf, uint32_t bufLen)
{
    static const unsigned long crcTable[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535,
        0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD,
        0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D,
        0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
        0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4,
        0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
        0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC,
        0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
        0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F,
        0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB,
        0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
        0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA,
        0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE,
        0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A,
        0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409,
        0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
        0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739,
        0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
        0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268,
        0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0,
        0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8,
        0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF,
        0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703,
        0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
        0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
        0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE,
        0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
        0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6,
        0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D,
        0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5,
        0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605,
        0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
        0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };
    unsigned long crc32;
    unsigned char *byteBuf;
    size_t i;

    /** accumulate crc32 for buffer **/
    crc32 = inCrc32 ^ 0xFFFFFFFF;
    byteBuf = (unsigned char *)buf;

    for (i = 0; i < bufLen; i++) {
        crc32 = (crc32 >> 8) ^ crcTable[(crc32 ^ byteBuf[i]) & 0xFF];
    }

    return (crc32 ^ 0xFFFFFFFF);
}
