
#include “flexcan_mpc5554.h”
uint32_t RxCODE; /* Received message buffer code */
uint32_t RxID; /* Received message ID */
uint32_t RxLENGTH; /* Received message number of data bytes */
uint8_t RxDATA[8]; /* Received message data string*/
uint32_t RxTIMESTAMP; /* Received message time */

void initCAN_A (void) {
uint8_t i;
//CAN_A.MCR.R = 0x5000003F; /* Put in Freeze Mode amp enable all 64 msg buffers*/
CAN_A.MCR.R = 0x1000003F; /* Put in Normal amp enable all 64 msg buffers*/ /* Normal Mode*/

// while(CAN_A.MCR.B.FRZ == 0) { } ;

CAN_A.CR.R = 0x04DB0006; /* Configure for 8MHz OSC, 100kHz bit time */

for (i=0; ilt64; i++) {
CAN_A.BUF[i].CS.B.CODE = 0; /* Inactivate all message buffers */
}

CAN_A.BUF[0].CS.B.CODE = 8; /* Message Buffer 0 set to TX INACTIVE */

SIU.PCR[83].R = 0x062C; /* MPC555x: Configure pad as CNTXA, open drain */
SIU.PCR[84].R = 0×0500; /* MPC555x: Configure pad as CNRXA */
CAN_A.MCR.R = 0x0000003F; /* Negate FlexCAN A halt state for 64 MB */

//while (CAN_A.MCR.B.FRZACK == 0) {};

}
void initCAN_B (void) {
uint8_t i;

//CAN_B.MCR.R = 0x5000003F; /* Put in Freeze Mode amp enable all 64 msg buffers*/
CAN_B.MCR.R = 0x1000003F; /* Put in Normal amp enable all 64 msg buffers*/ /*Normal Mode*/

CAN_B.CR.R = 0x04DB0006; /* Configure for 8MHz OSC, 100kHz bit time */

for (i=0; ilt64; i++) {
CAN_B.BUF[i].CS.B.CODE = 0; /* Inactivate all message buffers */
}

CAN_B.BUF[4].CS.B.IDE = 0; /* MB 4 will look for a standard ID */
CAN_B.BUF[4].ID.B.STD_ID = 555; /* MB 4 will look for ID = 555 */
CAN_B.BUF[4].CS.B.CODE = 4; /* MB 4 set to RX EMPTY */
CAN_B.RXGMASK.R = 0x1FFFFFFF; /* Global acceptance mask */

SIU.PCR[87].R = 0x0E2C; /* MPC555x: Configure pad as CNTXC, open drain */
SIU.PCR[88].R = 0x0D00; /* MPC555x: Configure pad as CNRXC */
CAN_B.MCR.R = 0x0000003F; /* Negate FlexCAN B halt state for 64 MB */
}

void TransmitMsg (void) {
uint8_t i;

/* Assumption: Message buffer CODE is INACTIVE */
const uint8_t TxData[] = {“Hello World!! \n\r”}; /* Transmit string*/

CAN_A.BUF[0].CS.B.IDE = 0; /* Use standard ID length */
CAN_A.BUF[0].ID.B.STD_ID = 555; /* Transmit ID is 555 */
CAN_A.BUF[0].CS.B.RTR = 0; /* Data frame, not remote Tx request frame */
CAN_A.BUF[0].CS.B.LENGTH = sizeof(TxData) -1 ; /* # bytes to transmit w/o null */

for (i=0; iltsizeof(TxData); i++) {
CAN_A.BUF[0].DATA.B[i] = TxData[i]; /* Data to be transmitted */
}

CAN_A.BUF[0].CS.B.SRR = 1; /* Tx frame (not req’d for standard frame)*/
CAN_A.BUF[0].CS.B.CODE =0xC; /* Activate msg. buf. to transmit data frame */
}
void ReceiveMsg (void) {
uint8_t j;
uint32_t dummy;

while (CAN_B.IFRL.B.BUF04I == 0) {}; /* MPC555x: Wait for CAN B MB 4 flag */

RxCODE = CAN_B.BUF[4].CS.B.CODE; /* Read CODE, ID, LENGTH, DATA, TIMESTAMP*/
RxID = CAN_B.BUF[4].ID.B.STD_ID;
RxLENGTH = CAN_B.BUF[4].CS.B.LENGTH;

for (j=0; jltRxLENGTH; j++) {
RxDATA[j] = CAN_B.BUF[4].DATA.B[j];
}

RxTIMESTAMP = CAN_B.BUF[4].CS.B.TIMESTAMP;
dummy = CAN_B.TIMER.R; /* Read TIMER to unlock message buffers */

/* Use 1 of the next 2 lines: */
/*CAN_B.IFLAG1.R = 0×00000010;*/ /* MPC551x: Clear CAN C MB 4 flag */

CAN_B.IFRL.R = 0×00000010; /* MPC555x: Clear CAN C MB 4 flag */
}

void FlexCAN_program()
{
volatile uint32_t IdleCtr = 0;
initCAN_B(); /* Initialize FLEXCAN B amp one of its buffers for receive*/
initCAN_A(); /* Initialize FlexCAN A amp one of its buffers for transmit*/
TransmitMsg(); /* Transmit one message from a FlexCAN A buffer */
ReceiveMsg(); /* Wait for the message to be received at FlexCAN B */
while (1) { /* Idle loop: increment counter */
IdleCtr++;
}

}
