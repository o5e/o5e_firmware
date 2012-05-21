
#ifndef EXTERN
#define EXTERN extern
#define NOINIT
#endif

#include "cpu.h"

// list of command Qs - count must be exactly right
EXTERN uint32_t ADC_CmdQ0[40];		// 
EXTERN uint32_t ADC_CmdQ1[1];		
EXTERN uint32_t ADC_CmdQ2[1];		// unused
EXTERN uint32_t ADC_CmdQ3[1];		// Knock from eTPU 28 AD0/AD1 and AD2/AD3 (both differential)
EXTERN uint32_t ADC_CmdQ4[1];   	// unused
EXTERN uint32_t ADC_CmdQ5[1];   	// for MAP based on trigger from eTPU 26 AD18 

// where the results are stored, 
// count must be exactly right, usually same as above (unless using time stamps)
EXTERN vuint16_t ADC_RsltQ0[40];
EXTERN vuint16_t ADC_RsltQ1[1];
EXTERN vuint16_t ADC_RsltQ2[1];
EXTERN vuint16_t ADC_RsltQ3[1];
EXTERN vuint16_t ADC_RsltQ4[1];
EXTERN vuint16_t ADC_RsltQ5[1];

void init_ADC(void);
void Stop_ADC(void);
void initADC0 (void);
uint16_t Read_AD (uint_fast8_t channel);

#undef EXTERN
