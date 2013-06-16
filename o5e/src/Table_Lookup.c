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
Does range and error checks
ISO C90 compatible (yuck, check with "gcc -Wall -pedantic -Wextra")

To use this, you need to fill out a table structure and then pass the lookup 
routine a pointer to the structure.

Notes: a 1D table means that rows = 1.  The origin is the upper left and axis values must increase or 
stay the same as you go down (rows) or to the right (cols).

****************************************************************************************************/

/* Copyright (c) 2011, 2012 Jon Zeeff */

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
float table_lookup(const float x, const float y, const struct table * const table)
{
	uint8_t x_index;
	uint8_t y_index; 
	float value1;
	float value2;
	float value3;
	float value4;
	float ratio;
	uint8_t i;
	float *ptr;
#define float_size	2  	/* Shifts required to correct for 4 byte long 32bit float data	*/

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
		if (table->x_axis[i] < table->x_axis[i - 1]) {
			err_push( CODE_OLDJUNK_FB );
			return 15;
		}
	}
	/* check for proper y axis sorting (must be ascending) */
	for (i = 1; i < rows; ++i) {
		if (table->y_axis[i] < table->y_axis[i - 1]) {
			err_push( CODE_OLDJUNK_FA );
			return 15;
		}
	}
#endif

	/* find  indexes that point into the table */
	// check for within table
	if (x <= table->x_axis[0])              /* < first */ 
		x_index = 0; 
	else if (x >= table->x_axis[cols-1])   /* > last */ 
		x_index = cols - 1;
	else   /* In the table somewhere...  */
		x_index = bsearch(x, table->x_axis, cols);	/* Find the closest x index in the table

	/* do y axis if this is a 2D lookup */
	if ( y <= table->y_axis[0])			/* < first value */
		y_index = 0;
	else if ( y >= table->y_axis[rows-1])		/* > last value */
		y_index = rows - 1;
	else
		y_index = bsearch(y, table->y_axis, rows);


	/* Interpolate between the four nearest cells (2D) using bilinear method */

	/* Make sure we are not above or below the columns */
	if ( (x > table->x_axis[x_index] && x_index >= cols -1) || ( x < table->x_axis[x_index] && x_index == 0))
	{		/* Left or right most column, no interpolation on x */
		if ( (y > table->y_axis[y_index] && y_index >= rows - 1) || ( y < table->y_axis[y_index] && y_index == 0) )
		{ /* No interprolation on y either */
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * y_index) + x_index) << float_size);
			return value(ptr);
		}
		else
		{
			/* No interporlation on x, but we are interpolating on y */
			ratio = (y - table->y_axis[y_index])/(table->y_axis[y_index + 1] - table->y_axis[y_index]);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * y_index) + x_index) << float_size);
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * (y_index + 1)) + x_index) << float_size);
			value2 = value(ptr);
			return interpolate(ratio, value1, value2);
		}
	}
	else /* We are interpolating on x */
	{
		if ( (y > table->y_axis[y_index] && y_index >= rows - 1) || ( y < table->y_axis[y_index] && y_index == 0) )
		{ /* No interprolation on y, but we are still interpolating on x */
			ratio = (x - table->x_axis[x_index])/(table->x_axis[x_index + 1] - table->x_axis[x_index]);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * y_index) + x_index) << float_size);
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * y_index) + (x_index + 1)) << float_size);
			value2 = value(ptr);
			return interpolate(ratio, value1, value2);
		}
		else /* Most complex case, we are interpolating on both x and y... */
		{
			/* Interpolate along current y row first */
			ratio = (x - table->x_axis[x_index])/(table->x_axis[x_index + 1] - table->x_axis[x_index]);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * y_index) + x_index) << float_size);
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * y_index) + (x_index + 1)) << float_size);
			value2 = value(ptr);
			value3 = interpolate(ratio, value1, value2);

			/* Interpolate along next y row */
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * (y_index + 1)) + x_index) << float_size);
			value1 = value(ptr);
			/* Calculate the correct pointer location for our data point */
			ptr = (float *) table->data + (((cols * (y_index + 1)) + (x_index + 1)) << float_size);
			value2 = value(ptr);
			value4 = interpolate(ratio, value1, value2);

			/* Now we can interpolate along the x column to get our final value */
			ratio = (y - table->y_axis[y_index])/(table->y_axis[y_index + 1] - table->y_axis[y_index]);
			return interpolate(ratio, value3, value4); /* Return our final result */
		}
	}
} /* table_lookup() */
