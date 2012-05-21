/**********************************************************************************************

   @file   Packet.c
   @author Jon Zeeff 
   @date   October, 2011
   @brief  Open5xxxECU - handles serial port packets to/from tuner
   @note   Designed to be compatiable with MegaTunix
   @note   No existing code I've seen was good enough to use so any vague resemblance to any other
   @note   code is due to the compatibility goal and falls under "fair use"

   @version 1.0
   REV    AUTHOR        DATE          DESCRIPTION OF CHANGE                       
   ---    -----------   ----------    -------------------                         
   .1    J. Zeeff      13/Oct/11     initial version - in progress, not tested

**********************************************************************************/
/*

Copyright (c) 2011 Jon Zeeff
This is restricted by the JZ License.  NO copying, viewing, distribution, nothing is allowed except with express permission from the author specifically stating that YOU have permission.  If you have a copy of this file without proof of that permission, then you have 
it illegally and need to immediately delete all copies and then turn yourself in to the proper authorities.

At some point, this will have the MIT license.

*/

#include "config.h"
#include "main.h"
#include "Tuner_OPS.h"
#include "Packet.h"
#include "eSCI_DMA.h"
#include "Serial.h"
#include <string.h>
#include "OS.h"

static uint8_t tmp_buf[SERIAL_BUFFER_SIZE];     // just to hold it while we process it

// decipher a packet and execute the command inside
// format: header flags, command, optional sequence #, optional length
// use a format compatible with existing tuners with minimal changes
// See MegaTunix for info on the protocol structure

#define HAS_LENGTH 	0x1         // bit flags for headers
#define ACK_TYPE 	0x2
#define HAS_SEQUENCE 	0x4

uint32_t tuner_packet_count;		// for debug

// given a complete input packet, process the command in it
// Return number of bytes to be sent as response
// Put output in the packet that was used as input

int process_packet(uint8_t *packet, uint16_t size) 
{
uint8_t flags;
uint16_t command;
uint8_t seq;
uint16_t length;
pointers ptr;

ptr.uint8 = packet;

++tuner_packet_count;		// track quantity

// get header flags - first byte
flags = *ptr.uint8++;
// get command - two bytes
command = *ptr.uint16++;
// get sequence if present - one byte
if (flags & HAS_SEQUENCE) 
   seq = *ptr.uint8++;
// get length if present - two bytes, needed for variable size packets
if (flags & HAS_LENGTH) 
   length = *ptr.uint16++;

// Info about a specific ID, eg. address and length are taken from the TableOfTables

// Commands
// Even is tuner to ECU,  response (to tuner) is one less
// Change these to more compatible values if you want to
#define FirmwareVersion 0x2
#define IDList   		0xDA5E  // get list of all available objects
#define IDInfo   		0xF8E0  // get object info like flags, parent, address and length
#define Reset			0x12    // reset CPU
#define WriteFlash		0x18    // writes all tables to flash
#define Log				0x20
#define ReadMemory16 	0x258   // read a byte at a 16 bit memory address
#define OneD_Cell		0x012C  // write a 1D cell
#define TwoD_Cell		0x1324  // write a 2D cell
#define Errors			0x24
#define ReadbyID		0x26    // send table to tuner
#define WritebyID		0x28    // writes table to ram
// more to be added

#define VERSION "O5E V1.0"
#define MAX_READ_MEM 100    // TODO: pick a real value

switch (command) {
#if 0
    case ReadMemory16:
	 // extract length requested - 2 bytes
         length = *ptr.uint16++; 
         if (length > MAX_READ_MEM)
            break;
         // extract address requested - 2 bytes
         addr = *ptr.uint16++;
         // create output packet
         // address is offset from ram base
         memcpy();
         // send output packet
         break;
    case Command_List:
              // type - 1 byte, 0,1,2
              // attribs mask - 2 bytes
              break;
#endif
    case FirmwareVersion:
         {
         uint8_t buf[100];
         uint16_t len=0;
         pointers ptr;

         // send output packet containing version string
         ptr.uint8 = buf;   
         *ptr.uint8++ = HAS_LENGTH;				// flags
         *ptr.uint16++ = FirmwareVersion - 1;   // command
         *ptr.uint16++ = strlen(VERSION) + 1;	// length
         strcpy((char *)ptr.uint8,VERSION);		// version string

         len = packetize_output(buf, packet, ptr.uint8 - buf );   // write output to input packet
 
         return len;
         
         }
         break;
    default: 
         break;
} // switch

return 0;

} // process_packet()
