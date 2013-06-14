
#ifndef Table_Lookup_h
#define Table_Lookup_h

/* size of biggest table we will use - max 255 */
#define MAX_ROWS 32
#define MAX_COLS 32

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
struct table
{
	table_info;
	uint8_t data[MAX_ROWS*MAX_COLS];   	/* rows*cols array of ints, X order, rows first */
};

int table_lookup (const int x, const int y, const struct table * const t);

#endif
