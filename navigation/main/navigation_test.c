
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>
#include "navigation_run.h"


// Exercise 1
// Given a function, bit_shift_right, that takes an integer and returns an integer with its bits
// shifted to the right by one,
// write a unit tests that checks is the function is working properly when the integer 5 is passed to the function.

Describe(BitShiftRight);
BeforeEach(BitShiftRight) {}
AfterEach(BitShiftRight) {}

Ensure(BitShiftRight, shiftsBitRight) {
	int number = 5;
	
	// Your code goes here.
}


/* Exercise 2
 * Write a unit tests a function that takes an integer and returns the integer
 * with the bits shifted to the left.
 * 
 * After the unit test has been written, write the function that performs this operation.

*/

int main(int argc, char** argv) {

	return 0;
}

