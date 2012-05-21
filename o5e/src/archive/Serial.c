
/**********************************************************************************************
   @file   Serial.c
   @author Jon Zeeff 
   @date   October, 2011
   @brief  Open5xxxECU - handles serial port packets to/from tuner
   @note   Designed to be compatiable with portions of MegaTunix written by Perry Harrington
   @note   New code - any vague resemblance to any other
   @note   code is due to the compatibility goal and falls under "fair use"

   @version 1.0
   REV    AUTHOR        DATE          DESCRIPTION OF CHANGE                       
   ---    -----------   ----------    -------------------                         
   .1    J. Zeeff      13/Oct/11     initial version - in progress, not tested

**********************************************************************************/
/*

Copyright (c) 2011 Jon Zeeff
This is restricted by the JZ License.  NO copying, viewing, distribution, nothing is allowed except with 
express permission from the author specifically stating that YOU have permission.  If you have a copy 
of this file without proof of that permission, then you have it illegally and need to immediately delete 
all copies and then turn yourself in to the proper authorities.

At some point, this will have the MIT license.

*/

#include "config.h"
#include "main.h"
#include "eDMA_OPS.h"
#include "Tuner_OPS.h"
#include "Serial.h"
#include "Packet.h"
#include "eDMA_OPS.h"
#include <string.h>
#include "OS.h"

// These six values constitute "fair use" from MTX
// You too can do anything you want with them without any concern about licences
#define START_BYTE       0xAA
#define ESC_BYTE         0xBB
#define END_BYTE         0xCC
#define ESC_START_BYTE   0x55
#define ESC_ESC_BYTE     0x44
#define ESC_END_BYTE     0x33

// See rfc1055 for the basic encapsulation protocol implemented below

static uint8_t esc_next = 0;	// state flag indicates that previous char was the escape indicator

// unescape bytes and process packet when we have a complete one
// ie, deserialize and packetize the stream
// return n if the packet is complete, otherwise 0
// Note: uses a global packet[] and packet_ptr!


int process_serial_byte(uint8_t c)
{
	if (packet_ptr == packet + sizeof(packet)) {	// packet too big - dump it
		packet_ptr = packet;
		esc_next = 0;
                system_error(53829, __FILE__, __LINE__, "");
		return 0;
	}

	if (esc_next) {	// previous character was escape indicator
		esc_next = 0;
		switch (c) {
		case ESC_ESC_BYTE:
			*packet_ptr++ = ESC_BYTE;
			break;
		case ESC_START_BYTE:
			*packet_ptr++ = START_BYTE;
			break;
		case ESC_END_BYTE:
			*packet_ptr++ = START_BYTE;
			break;
		default:			// escape the unknown
			packet_ptr = packet;
			esc_next = 0;
                        system_error(43842, __FILE__, __LINE__, "");
		}
		return 0;
	}

	switch (c) {
	case ESC_BYTE:
		esc_next = 1;
		break;
	case START_BYTE:
		packet_ptr = packet;		// clear buffer
		esc_next = 0;
		break;
	case END_BYTE:			// we have a full packet, process it
        {
		uint16_t size = packet_ptr - packet;
                packet_ptr = packet;                           // reset for next packet
		if (size < 4) 				// too small to be legit
                   system_error(13729, __FILE__, __LINE__,"");
		else {
                        --size;				// drop checksum at the end
			uint8_t sum = checksum(packet,size) & 0xff;     
			if (sum != packet[size]) 	// verify 8 bit checksum 
                            system_error(23929, __FILE__, __LINE__,"");
			else 
			   return size;			// valid packet
		}
        }
                return 0;
		break;
	default:		// add byte to buffer
		*packet_ptr++ = c;
	}
        return 0;
}				// process_serial_byte()

// Copy in to out with escapes and checksum
// Note that out could be more than twice as big as in
// See rfc1055 for the basic encapsulation protocol implemented below

uint16_t packetize_output(uint8_t *in, uint8_t *out, uint16_t len)
{
uint8_t *start = out;			    // save for size calc
out[len] = checksum(in,len);   // append checksum - note, expands input buffer
++len;

while (len--) {
	switch (*in) {
	case ESC_BYTE:
                *out++ = ESC_BYTE;
                *out++ = ESC_ESC_BYTE;
		break;
	case START_BYTE:
                *out++ = ESC_BYTE;
                *out++ = ESC_START_BYTE;
		break;
	case END_BYTE:	
                *out++ = ESC_BYTE;
                *out++ = ESC_END_BYTE;
		break;
        default:
                *out++ = *in;
        }
        ++in;
}

return out-start;  	// return size of output packet

}

