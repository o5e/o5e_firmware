
// items related to support for the tuner

#ifndef EXTERN
#define EXTERN extern
#endif

#define MAX_PACKET_SIZE 5000
EXTERN void tuner_task(void);
EXTERN uint8_t packet[MAX_PACKET_SIZE];
EXTERN uint8_t *packet_ptr;
EXTERN uint16_t checksum(const uint8_t *addr, const uint_fast16_t count);
uint8_t Find_Empty_Page(uint8_t block);
uint8_t Page_Is_Blank(uint8_t *ptr);
uint8_t Burn_Page(uint8_t block,  uint_fast16_t page, uint_fast16_t dest_page);
int32_t make_packet(uint8_t code, const void *buf, uint16_t size);
int16_t check_crc(uint8_t * packet);
uint32_t Crc32_ComputeBuf(uint32_t inCrc32, const void *buf, uint32_t bufLen);
void tuner_task(void);
int32_t process_packet(const uint8_t * buf, uint16_t size);
void _start(void);

#undef EXTERN
