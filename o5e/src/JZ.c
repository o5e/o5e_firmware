
// Personal stuff - not for general use
// Jon Zeeff

#include "config.h"
#include "system.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include "eSCI_DMA.h"

#ifdef JZ

// uses TP, but might use MAP for cam position

void initADC0(void);
uint32_t Read_AD(uint_fast8_t channel);
void generator(void);

vint32_t value;
vint32_t prev_value;
vint32_t prev_prev_value;
vint32_t peak;
uint32_t start_time;
uint32_t rpm;
uint32_t prev_clock;

// assume a 1/2 wave rectified voltage on the A/D input

void generator(void)
{
    unsigned char string[100];

    initADC0();
    while (1) {
        // wait for inflection point - falling to level
        prev_prev_value = prev_value = 0;
        while (1) {
            prev_clock = clock();
            value = Read_AD(39);
            // look for falling edge then leveling off near zero
            // could look for rising edge
            if (value < 400 && prev_value >= 400)
                break;
            prev_prev_value = prev_value;
            prev_value = value;
            fuel_pulse();
        }
        // record time
        start_time = clock();
        // delay
        while (clock() - start_time < TICKS_PER_MSEC * 4) {
            fuel_pulse();
        };

        // record max until next inflection point
        peak = 0;
        prev_prev_value = prev_value = 0;
        while (1) {
            prev_clock = clock();
            value = Read_AD(39);
            // look for falling edge then leveling off near zero
            if (value < 400 && prev_value >= 400 && peak > 600)
                break;
            prev_prev_value = prev_value;
            prev_value = value;
            if (value > peak)
                peak = value;   // track max
            fuel_pulse();
        }
        // calc RPM from times of inflection points
        rpm = ((60000) * TICKS_PER_MSEC) / ((clock() - start_time) & 0xffffff);
        sprintf(string, "%u end at %u, rpm = %u  %u\r\n", start_time, clock(),
                rpm, peak / 24);
        send_serial_A(string, strlen(string));
        // delay - why?
        while (clock() - start_time < TICKS_PER_MSEC * 4) {
            fuel_pulse();
        };

        // calc and set new throttle position based on peak voltage seen or rpm
        // use PID
        tp = 1;                 // PWM output

        // calc fuel based on throttle position and rpm table
        // rpm is almost always 3600
        pw = table_lookup_jz(rpm, tp, (struct table_jz *)&fuel_table);

        // schedule next pulse TODO - every other pair of pulses?
        pulse_start = clock();
        pulse_end = pulse_start + pw;

    }
}

// check for time to turn on or off injector
fuel_pulse()
{
    if (clock() >= pulse_start && clock() <= pulse_end) // TODO rollover
        turn_on;
    else
        turn_off;
}

void initADC0(void)
{
    EQADC.CFPR[0].R = 0x80800001;       /* Send CFIFO 0 a ADC0 configuration command */
    /* enable ADC0 & sets prescaler= divide by 2 */
    EQADC.CFCR[0].R = 0x0410;   /* Trigger CFIFO 0 using Single Scan SW mode */
    while (EQADC.FISR[0].B.EOQF != 1) {
    }                           /* Wait for End Of Queue flag */
    EQADC.FISR[0].B.EOQF = 1;   /* Clear End Of Queue flag */
}

uint32_t Read_AD(uint_fast8_t channel)
{
    uint32_t Result;

    /* SendConvCmd  */
    EQADC.CFPR[0].R = 0x80000000 | (channel << 8);      /* Conversion command: convert channel n */
    /* with ADC0, set EOQ, and send result to RFIFO 0 */
    EQADC.CFCR[0].R = 0x0410;   /* Trigger CFIFO 0 using Single Scan SW mode */

    while (EQADC.FISR[0].B.RFDF != 1) {
    }                           /* Wait for RFIFO 0's Drain Flag to set */

    Result = EQADC.RFPR[0].R;   /* ADC result */
    Result = (uint32_t) ((5000 * Result) / 0x3FFC);     /* ADC result in millivolts */
    EQADC.FISR[0].B.RFDF = 1;   /* Clear RFIFO 0's Drain Flag */
    EQADC.FISR[0].B.EOQF = 1;   /* Clear CFIFO's End of Queue flag */
    return Result;
}

void code_speed(void)
{
    asm {
// record the execution time of some code
#INITIALIZE TIME BASE=0
        li r4, 0
#Load immediate data of 0 to r4
         mttbu r4
#Move r4 to TBU
         mttbl r4
#Move r4 to TBL
#ENABLE TIME BASE
         mfhid0 r5
#Move from spr HID0 to r5 (copies HID0)
         li r4, 0x4000
#Load immed. data of 0x4000 to r4
         or r5, r4, r5
#OR r4 (0x0000 4000) with r5 (HID0 value)
         mthid0 r5
#Move result to HID0
#EXECUTE SOME CODE
         nop nop nop nop nop
#RECORD TBL
         mftbl r5
#Move TBL to r5 to store TBL value
}}                              // code_speed()
#endif
