/**************************************************************************************************

@file   	table_lookup.c
@author 	Jon Zeeff
@date   	September, 2011

@author		Clint Corbin
@date			June 2013

@brief  	table lookup - fast, generic, 1D, 2D, variable axis, float, binary search 
@copyright 	MIT license
@version 	1.8

@brief		updated to support variable length floating point 1D or 2D tables only

@note Generic, portable 1D or 2D table lookup
Table entries are float (32 bit single precision IEEE 754)
Uses a binary search for the variable axis increments (faster)
Logs errors if desired
Does range and error checks.  Error codes changed to negative numbers.
ISO C90 compatible (yuck, check with "gcc -Wall -pedantic -Wextra")

To use this, you need to fill out a table structure and then pass the lookup 
routine a pointer to the structure.

Notes: a 1D table means that rows = 1.  The origin is the upper left and axis values must increase or 
stay the same as you go down (rows) or to the right (cols).

****************************************************************************************************/

/* Copyright (c) 2011, 2012 Jon Zeeff 
   Copyright (c) 2013 Clint Corbin
   Copyright (c) 2013 Mark Eberhardt				*/

#include <stdint.h>
#include "Table_Lookup.h"
#include "err.h"

#ifndef FALSE
#   define FALSE 0
#   define TRUE 1
#endif


/* generic binary search for a int8_t array */
/* does not do bounds checking */

static inline uint8_t bsearch(const float value, const float * const array, uint8_t n)
{
	uint8_t lower;
	uint8_t middle;
	uint8_t upper;

	lower = 0;
	upper = --n;                /* note, n-1 is now max index, zero based arrays */

	for (uint8_t i = 0; i <= n; i++)	/* I REALLY hate infinite loops! */
	{
		middle = (upper + lower) / 2;   /* 1/2 way between them */

		if (middle == lower)
			break;       	/* done */

		if (array[middle] < value)
			lower = middle;
		else if (array[middle] > value)
			upper = middle;
		else				/* equal */
			break;				/* done */
	}
	return middle;     

} // bsearch()

/* Linear interpolation between two points on a line. */
inline static float interpolate(const float fraction, const float value1, const float value2)
{
	if (value1 == value2 || fraction == 0)      /* for speed, no divisions required */
		return value1;

	if (value1 < value2)        /* postitive slope */
		return value1 + ((value2 - value1) * fraction);
	else
		return value1 - ((value1 - value2) * fraction);
}


/************************************************************************

@param x value
@param y value (optional)
@param pointer to table structure
@return lookup value from table

************************************************************************/


float table_lookup(const float col_value, const float row_value, const struct table * const table)
{	
	uint8_t col_index;
	uint8_t row_index; 
	float value1;
	float value2;
	float value3;
	float value4;
	float ratio;
	uint8_t i;
	float *ptr;


	/* for readability */
#define rows (table->rows)
#define cols (table->cols)

#define PARANOIA
#ifdef PARANOIA

	/* check for sane values */
	if (table == 0 || rows < 1 || cols < 2 || rows > MAX_ROWS || cols > MAX_COLS) {     /* error check */
		err_push( CODE_OLDJUNK_FC );
		return 255;
	}
	/* check for proper x axis sorting (must be ascending) */
	for (i = 1; i < cols; ++i) {
		if (table->col_axis[i] < table->col_axis[i - 1]) {
			err_push( CODE_OLDJUNK_FB );
			return 15;
		}
	}
	/* check for proper y axis sorting (must be ascending) */
	for (i = 1; i < rows; ++i) {
		if (table->row_axis[i] < table->row_axis[i - 1]) {
			err_push( CODE_OLDJUNK_FA );
			return 300;
		}
	}
#endif

	/* find  indexes that point into the table */
	// check for within table
	
	if (row_value <= table->row_axis[0])            			/* < first */ 
		row_index = 0; 
	else if (row_value >= table->row_axis[rows-1])   /* > last */ 
		row_index = rows - 1;
	else   /* In the table somewhere...  */
		row_index = bsearch(row_value, table->row_axis, rows);	/* Find the closest row_value index in the table*/

	/* do col_value axis if this is a 2D lookup */
	if ( col_value <= table->col_axis[0])						/* < first value */
		col_index = 0;
	else if ( col_value >= table->col_axis[cols-1])		/* > last value */
		col_index = cols - 1;
	else
		col_index = bsearch(col_value, table->col_axis, cols);

	/* Is this a 1D table? (1 col only) */
	if ( rows == 1 )  /* 1D only, so we only interpolate along the single row */
	{
		if ( col_value == table->col_axis[col_index] || col_index == cols - 1 ) /* No need to interpolate */
		{
			/* Calculate the correct pointer location for our data point */
            ptr = (float *) table->data + col_index ;
            return value(ptr);
			 
		}
		else
		{
			ratio = (col_value - table->col_axis[col_index])/(table->col_axis[col_index + 1] - table->col_axis[col_index]);
			ptr = (float *) ((unsigned int) table->data + col_index );
			value1 = value(ptr);
			ptr = (float *) ((unsigned int) table->data + col_index + 1);
			value2 = value(ptr);
			return interpolate(ratio, value1, value2);
		}
	}

	/* Interpolate between the four nearest cells (2D) using bilinear method */

	/* Make sure we are not above or below the columns */
	if ( (row_value > table->row_axis[row_index] && row_index >= rows -1) || ( row_value < table->row_axis[row_index] && row_index == 0))
	{		/* Upper or lower most row, no interpolation on row_value */
		if ( (col_value > table->col_axis[col_index] && col_index >= cols - 1) || ( col_value < table->col_axis[col_index] && col_index == 0) )
		{ /* No interprolation on col_value either */
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * row_index) + col_index) ;
			return value(ptr);
		}
		else
		{
			/* No interporlation on row_value, but we are interpolating on col_value */
			ratio = (col_value - table->col_axis[col_index])/(table->col_axis[col_index + 1] - table->col_axis[col_index]);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * row_index) + col_index);
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * (row_index + 1)) + col_index);
			value2 = value(ptr);
			return interpolate(ratio, value1, value2);
		}
	}
	else /* We are interpolating on row_value */
	{
		if ( (col_value > table->col_axis[col_index] && col_index >= cols - 1) || ( col_value < table->col_axis[col_index] && col_index == 0) )
		{ /* No interprolation on col_value, but we are still interpolating on row_value */
			ratio = (row_value - table->row_axis[row_index])/(table->row_axis[row_index + 1] - table->row_axis[row_index]);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * row_index) + col_index);
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * row_index) + (col_index + 1));
			value2 = value(ptr);
			return interpolate(ratio, value1, value2);
		}
		else /* Most complex case, we are interpolating on both row_value and col_value... */
		{
			/* Interpolate along current col_value row first */
			ratio = (row_value - table->row_axis[row_index])/(table->row_axis[row_index + 1] - table->row_axis[row_index]);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * row_index) + col_index) ;
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * (row_index + 1)) + col_index);
			value2 = *ptr;
			value3 = interpolate(ratio, value1, value2);

			/* Interpolate along next col_value row */
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * (row_index + 1)) + col_index);
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + ((cols * (row_index + 1)) + (col_index + 1));
			value2 = value(ptr);
			value4 = interpolate(ratio, value1, value2);

			/* Now we can interpolate along the row_value column to get our final value */
			ratio = (col_value - table->col_axis[col_index])/(table->col_axis[col_index + 1] - table->col_axis[col_index]);
			return interpolate(ratio, value3, value4); /* Return our final result */
		}
	}
} /* table_lookup() */
