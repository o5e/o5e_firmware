
// FIR routine written by FreeScale
// 10 taps, assembler is 10x faster than below C code
// AN3509

// 10-TAP FIR filter written in C-code for performance comparison.
// N is the number of samples, h[] is a vector of coefficients, x[] is the input vector and y[] is output

void fir10c(int N, int *x_ptr, short int *y_ptr, int *h_ptr)
{
        int m, n, accumulator = 0;
        for (n = 0; n < N; n++) {
                for (m = 0; m < 10; m++) 
                        accumulator = accumulator + (*(h_ptr + m) * *(x_ptr + m));
                *(y_ptr + n) = accumulator;
                accumulator = 0;
                x_ptr = x_ptr + 1;
        }; // for
}


void fir10(int N, int *x_ptr, short int *y_ptr, int *h_ptr)
{
asm {

// r3 = N
// r4 = x[]
// r5 = y[]
// r6 = h[]

//--------------------------------------------------------------------------
// Load coeffs into registers
//--------------------------------------------------------------------------

 evlwhou    r16, 0(r6);	 // r16 = Coefficient[1]
 evlwhou    r17, 4(r6);	 // r17 = Coefficient[2]
 evlwhou    r18, 8(r6);	 // r18 = Coefficient[3]
 evlwhou    r19, 12(r6);	 // r19 = Coefficient[4]
 evlwhou    r20, 16(r6);	 // r12 = Coefficient[5]
 evlwhou    r21, 20(r6);	 // r21 = Coefficient[6]
 evlwhou    r22, 24(r6);	 // r21 = Coefficient[7]
 evlwhou    r23, 28(r6);	 // r23 = Coefficient[8]
 evlwhou    r24, 32(r6);	 // r24 = Coefficient[9]
 evlwhou    r25, 36(r6);	 // r25 = Coefficient[10]

//--------------------------------------------------------------------------
// Initialize counter to zero
//--------------------------------------------------------------------------

 li r15, 0x0 // use r15 as a counter

//--------------------------------------------------------------------------
// Load first set of data to be processed
//--------------------------------------------------------------------------

 evlwhou        r26, 0(r4);	// Load InputVector_0
 evlwhou        r27, 4(r4);	// Load InputVector_1
 evlwhou        r28, 8(r4);	// Load InputVector_2
 evlwhou        r29, 12(r4);	// Load InputVector_3
 evlwhou        r30, 16(r4);	// Load InputVector_4

//--------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------

loop:

 evlwhou        r31, 20(r4);	// Load InputVector
 evmhossfa      r6, r26, r25;	  // InputVector_0 * Coefficients 10
 evmhossfaaw    r6, r27, r23;	  // InputVector_1 * Coefficients 8
 evmhossfaaw    r6, r28, r21;	  // InputVector_2 * Coefficients 6 
 evmhossfaaw    r6, r29, r19;	  // InputVector_3 * Coefficients 4
 evmhossfaaw    r6, r30, r17;	  // InputVector_4 * Coefficients 2
 evmergelohi    r26, r26, r27;	 // InputVector0  = Merge InputVector_0 & InputVector_1
 evmergelohi    r27, r27, r28;	 // InputVector1  = Merge InputVector_1 & InputVector_2
 evmergelohi    r28, r28, r29;	 // InputVector2  = Merge InputVector_2 & InputVector_3
 evmergelohi    r29, r29, r30;	 // InputVector3  = Merge InputVector_3 & InputVector_4
 evmergelohi    r30, r30, r31;	 // InputVector4  = Merge InputVector_4 & InputVector_5
 evmhossfaaw    r6, r26, r24;	  // InputVector_0 * Coefficients 9 
 evmhossfaaw    r6, r27, r22;	  // InputVector_1 * Coefficients 7
 evmhossfaaw    r6, r28, r20;	  // InputVector_2 * Coefficients 5  
 evmhossfaaw    r6, r29, r18;	  // InputVector_3 * Coefficients 3
 evmhossfaaw    r6, r30, r16;	  // InputVector_4 * Coefficients 1Summary

// Store output

 evstwho        r6, 0(r5);	
 addi           r5, r5, 4;	  // Update pointer to output vector for next results
 evmergelohi    r26, r26, r27;	 // InputVector0  = Merge InputVector_0 & InputVector_1
 evmergelohi    r27, r27, r28;	 // InputVector1  = Merge InputVector_1 & InputVector_2
 evmergelohi    r28, r28, r29;	// InputVector2  = Merge InputVector_2 & InputVector_3
 evmergelohi    r29, r29, r30;	// InputVector3  = Merge InputVector_3 & InputVector_4
 evmergehilo    r30, r31, r31;	// InputVector4  = InputVector_5
 addi           r4, r4, 4;	// Update r4 to point to next input vector
 addi           r15, r15, 2;	// Update counter 2 points
 cmpw           r15, r3;	// check if loop is done
 bne            loop;	

}
} // fir10()

