
// items related to support for the tuner

#ifndef EXTERN
#define EXTERN extern
#endif

#define MAX_PACKET_SIZE 5000
EXTERN void tuner_task(void);
EXTERN uint8_t packet[MAX_PACKET_SIZE];
EXTERN uint8_t *packet_ptr;
EXTERN uint16_t checksum(const uint8_t *addr, const uint_fast16_t count);

// universal pointer
typedef union 
{
    uint8_t  *uint8;
    uint16_t *uint16;
    uint32_t *uint32;
    int8_t  *int8;
    int16_t *int16;
    int32_t *int32;
    unsigned char *uchar;
    void *voidptr;
} pointers;

#undef EXTERN
