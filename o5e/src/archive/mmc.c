// Written by Ed Waugh 2004, www.edwaugh.co.uk

// Also see: http://elm-chan.org/fsw/ff/00index_p.html

// for the original source, and hundreds more examples of PIC C code, see:
// http://www.microchipc.com/sourcecode/#mmc

int mmc_init();
int mmc_response(unsigned char response);
int mmc_read_block(unsigned long block_number);
int mmc_write_block(unsigned long block_number);
int mmc_get_status();


/************************** MMC Init **************************************/
/* Initialises the MMC into SPI mode and sets block size, returns 0 on success */

int mmc_init()
{
int i;

SETUP_SPI(SPI_MASTER | SPI_H_TO_L | SPI_CLK_DIV_4 | SPI_SS_DISABLED);

*0x94 |= 0x40;                          // set CKE = 1 - clock idle low
*0x14 &= 0xEF;                          // set CKP = 0 - data valid on rising edge

OUTPUT_HIGH(PIN_C2);                    // set SS = 1 (off)

for(i=0;i<10;i++)                       // initialise the MMC card into SPI mode by sending clks on
{
        SPI_WRITE(0xFF);
}

OUTPUT_LOW(PIN_C2);                     // set SS = 0 (on) tells card to go to spi mode when it receives reset

SPI_WRITE(0x40);                        // send reset command
SPI_WRITE(0x00);                        // all the arguments are 0x00 for the reset command
SPI_WRITE(0x00);
SPI_WRITE(0x00);
SPI_WRITE(0x00);
SPI_WRITE(0x95);                        // precalculated checksum as we are still in MMC mode

puts("Sent go to SPI\n\r");

if(mmc_response(0x01)==1) return 1;     // if = 1 then there was a timeout waiting for 0x01 from the mmc

puts("Got response from MMC\n\r");

i = 0;

while((i < 255) && (mmc_response(0x00)==1))     // must keep sending command if response
{
        SPI_WRITE(0x41);                // send mmc command one to bring out of idle state
        SPI_WRITE(0x00);                // all the arguments are 0x00 for command one
        SPI_WRITE(0x00);
        SPI_WRITE(0x00);
        SPI_WRITE(0x00);
        SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF
        i++;
}
if(i >= 254) return 1;                   // if >= 254 then there was a timeout waiting for 0x00 from the mmc

puts("Got out of idle response from MMC\n\r");

OUTPUT_HIGH(PIN_C2);                    // set SS = 1 (off)

SPI_WRITE(0xFF);                        // extra clocks to allow mmc to finish off what it is doing

OUTPUT_LOW(PIN_C2);                     // set SS = 0 (on)

        SPI_WRITE(0x50);                // send mmc command one to bring out of idle state
        SPI_WRITE(0x00);
        SPI_WRITE(0x00);
        SPI_WRITE(0x02);                // high block length bits - 512 bytes
        SPI_WRITE(0x00);                // low block length bits
        SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF

if((mmc_response(0x00))==1) return 1;
OUTPUT_HIGH(PIN_C2);            // set SS = 1 (off)
puts("Got set block length response from MMC\n\r");
return 0;
}

/************************** MMC Get Status **************************************/
/* Get the status register of the MMC, for debugging purposes */

int mmc_get_status()
{

OUTPUT_LOW(PIN_C2);                     // set SS = 0 (on)

        SPI_WRITE(0x58);                // send mmc command one to bring out of idle state
        SPI_WRITE(0x00);
        SPI_WRITE(0x00);
        SPI_WRITE(0x00);                //
        SPI_WRITE(0x00);                // always zero as mulitples of 512
        SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF

OUTPUT_HIGH(PIN_C2);                    // set SS = 1 (off)
return 0;
}

/************************** MMC Write Block **************************************/

int mmc_write_block(unsigned long block_number)
{
unsigned long i;
unsigned long varh,varl;

varl=((block_number&0x003F)<<9);
varh=((block_number&0xFFC0)>>7);

puts("Write block\n\r");                // block size has been set in mmc_init()

OUTPUT_LOW(PIN_C2);                     // set SS = 0 (on)

        SPI_WRITE(0x58);                // send mmc write block
        SPI_WRITE(HIGH(varh));
        SPI_WRITE(LOW(varh));
        SPI_WRITE(HIGH(varl));
        SPI_WRITE(0x00);                // always zero as mulitples of 512
        SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF

if((mmc_response(0x00))==1) return 1;
puts("Got response to write block\n\r");

SPI_WRITE(0xFE);                        // send data token

for(i=0;i<512;i++)
{

SPI_WRITE(i2c_eeprom_read(HIGH(i),LOW(i)));     // send data

}

SPI_WRITE(0xFF);                        // dummy CRC
SPI_WRITE(0xFF);

if((SPI_READ(0xFF)&0x0F)!=0x05) return 1;

puts("Got data response to write block\n\r");

OUTPUT_HIGH(PIN_C2);                    // set SS = 1 (off)
return 0;
}



/************************** MMC Read Block **************************************/
/**** Reads a 512 Byte block from the MMC and outputs each byte to RS232 ****/

int mmc_read_block(unsigned long block_number)
{
unsigned long i;
unsigned long varh,varl;

varl=((block_number&0x003F)<<9);
varh=((block_number&0xFFC0)>>7);

OUTPUT_LOW(PIN_C2);                     // set SS = 0 (on)

        SPI_WRITE(0x51);                // send mmc read single block command
        SPI_WRITE(HIGH(varh));      // arguments are address
        SPI_WRITE(LOW(varh));
        SPI_WRITE(HIGH(varl));
        SPI_WRITE(0x00);
        SPI_WRITE(0xFF);                // checksum is no longer required but we always send 0xFF

if((mmc_response(0x00))==1) return 1;   // if mmc_response returns 1 then we failed to get a 0x00 response (affirmative)

puts("Got response to read block command\n\r");

if((mmc_response(0xFE))==1) return 1;   // wait for data token

puts("Got data token\n\r");

        for(i=0;i<512;i++)
        {
                putc(SPI_READ(0xFF));               // we should now receive 512 bytes
        }

SPI_READ(0xFF);                 // CRC bytes that are not needed
SPI_READ(0xFF);

OUTPUT_HIGH(PIN_C2);            // set SS = 1 (off)
SPI_WRITE(0xFF);                // give mmc the clocks it needs to finish off

puts("\n\rEnd of read block\n\r");

return 0;
}

/************************** MMC get response **************************************/
/**** Repeatedly reads the MMC until we get the response we want or timeout ****/

int mmc_response(unsigned char response)
{
        unsigned long count = 0xFFFF;           // 16bit repeat, it may be possible to shrink this to 8 bit but there is not much point

        while(SPI_READ(0xFF) != response && --count > 0);

        if(count==0) return 1;                  // loop was exited due to timeout
        else return 0;                          // loop was exited before timeout
}
