/**
 * @file   eQADC_OPS.h
 * @author
 * @brief
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __eqadc_ops_h
#define   __eqadc_ops_h

#ifdef __cplusplus
extern "C"
{
#endif

/* TODO: figure out whats going on here. This code wasn't wrapped in a guard
         so I did it */

// list of command Qs - count must be exactly right
extern uint32_t ADC_CmdQ0[40];		// 
extern uint32_t ADC_CmdQ1[1];		
extern uint32_t ADC_CmdQ2[1];		// unused
extern uint32_t ADC_CmdQ3[1];		// Knock from eTPU 28 AD0/AD1 and AD2/AD3 (both differential)
extern uint32_t ADC_CmdQ4[1];   	// unused
extern uint32_t ADC_CmdQ5[1];   	// for MAP based on trigger from eTPU 26 AD18 

// where the results are stored, 
// count must be exactly right, usually same as above (unless using time stamps)
extern vuint16_t ADC_RsltQ0[40];
extern vuint16_t ADC_RsltQ1[1];
extern vuint16_t ADC_RsltQ2[1];
extern vuint16_t ADC_RsltQ3[1];
extern vuint16_t ADC_RsltQ4[1];
extern vuint16_t ADC_RsltQ5[1];

void init_ADC(void);

#ifdef __cplusplus
}
#endif

#endif // __eqadc_ops_h
