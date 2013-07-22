
#ifndef Table_Lookup_h
#define Table_Lookup_h

/* size of biggest table we will use - max 255 */
#define MAX_ROWS 32
#define MAX_COLS 32

/* Structure of the data table with header information */
									
struct table
{
	uint8_t cols;  			/* Number of columns in the table */
	uint8_t rows; 			/* Number of rows in the table */
	uint16_t filler;   			/*filler to put 32bit stuff on multiple of 4*/
	float col_axis[MAX_COLS];   /* Vector to store variable column coordinates */  
	float row_axis[MAX_ROWS]; 	/* Vector to store variable row coordinates */ 	                       
  						
	float data[MAX_ROWS * MAX_COLS];  /* rows*cols array of floats, X order, rows first */
};

/* Function Declarations */
float table_lookup ( const float col_value, const float row_value, const struct table * const t);

/*  macro to extract value from table */
#define value(index)	*(float *)index



#endif
