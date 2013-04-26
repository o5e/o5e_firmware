
#ifndef ENRICHMENT_OPS_H
#define ENRICHMENT_OPS_H
#ifdef __cplusplus
extern "C"
{
#endif
extern int32_t Accel_Decel_Corr;
extern uint32_t Prime_Corr;

void Get_Accel_Decel_Corr(void);
void Get_Prime_Corr(void);

#ifdef __cplusplus
}
#endif
#endif