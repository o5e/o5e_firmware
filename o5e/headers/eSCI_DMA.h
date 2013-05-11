/**
 * @file   <filename.h>
 * @author <author>
 * @brief  <one liner description>
 * @attention  { not for use in safety critical systems       }
 * @attention  { not for use in pollution controlled vehicles }
 *
 * (c) Copyright 2012, Sean Stasiak <sstasiak at gmail dot com>
 *
 */

#ifndef   __esci_dma_h
#define   __esci_dma_h

#ifdef __cplusplus
extern "C"
{
#endif

/* not sure whats going on here, but a guard was missing, so I added it */

// fast DMA versions 
void init_eSCI_DMA(void);
uint16_t write_serial(const uint8_t *bytes, const uint16_t count);
uint16_t read_serial(uint8_t *bytes, const uint16_t max_bytes);

#ifdef __cplusplus
}
#endif

#endif // __esci_dma_h



