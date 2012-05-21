
#ifndef Table_Lookup_JZ_h
#define Table_Lookup_JZ_h

#include <stdint.h>

/* alternate name */
#define Table_Lookup_JZ table_lookup_jz

/* size of biggest table we will use - max 255 */
#define MAX_ROWS 32
#define MAX_COLS 32

/* if you are using fixed axis spacing, then using these is more convenient */
#define min_x x_axis[0]
#define max_x x_axis[1]
#define min_y y_axis[0]
#define max_y y_axis[1]

/* this is the structure of a table header */
#define table_info 	\
	uint16_t rows;     \
	uint16_t cols; 	\
	int16_t x_axis[MAX_ROWS]; \
	int16_t y_axis[MAX_COLS];  \
	uint8_t variable_axis; \
	uint8_t byte_table;	\
	uint8_t filler[2]	

/* generic version */
struct table_jz
{
	table_info;
	uint8_t data[MAX_ROWS*MAX_COLS];   	/* rows*cols array of ints, X order, rows first */
};

int table_lookup_jz (const int x, const int y, const struct table_jz *t);
uint_fast16_t bsearch_jz (register const int16_t value, const int16_t * array, uint_fast16_t n);
unsigned lsearch_jz (register const int value, const int16_t * array, uint_fast16_t n);

/* specific versions */
/* add more below as needed for other sizes or types */

/* 3x3 table with unsigned 8 bit values */
struct table_3x3_u8
{
	table_info;
	uint8_t data[3][3];   	/* rows*cols array of ints, X order, rows first */
};

/* 3x3 table with signed 16 bit values */
struct table_3x3_16
{
	table_info;
	int16_t data[3][3];   	/* rows*cols array of ints, X order, rows first */
};

/* 16x16 table with signed 16 bit values */
struct table_16x16_16
{
	table_info;
	int16_t data[16][16];   	/* rows*cols array of ints, X order, rows first */
};

/* Add more here...... */

#endif
