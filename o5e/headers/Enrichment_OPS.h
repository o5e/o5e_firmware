
#ifndef ENRICHMENT_OPS_H
#define ENRICHMENT_OPS_H

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN int16_t Accel_Corr;                 // bin 12

extern uint8_t CLT_Corr, Post_Start_Corr;     
void Enrichment_Task();

#endif
#undef EXTERN
