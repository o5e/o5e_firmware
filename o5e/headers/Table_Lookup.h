
#ifndef Table_Lookup_h
#define Table_Lookup_h

/* size of biggest table we will use - max 255 */
#define MAX_ROWS 32
#define MAX_COLS 32

/* Structure of the data table with header information */
struct table
{
	uint8_t rows;											/* Number of rows in the table */
	uint8_t cols;											/* Number of columns in the table */
	float x_axis[MAX_ROWS];							/* Vector to store variable row coordinates */
	float y_axis[MAX_COLS];							/* Vector to store variable column coordinates */
	float data[MAX_ROWS*MAX_COLS];   		/* rows*cols array of floats, X order, rows first */
};

/* Function Declarations */
static inline uint8_t bsearch(const float value, const float * const array, uint8_t n);
float table_lookup (const float x, const float y, const struct table * const t);


/*  macro to extract value from table */
#define value(index)	*index

#endif
