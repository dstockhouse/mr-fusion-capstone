
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>
#include "navigation_run.h"

Describe(Strlen);
BeforeEach(Strlen) {}
AfterEach(Strlen) {}

Ensure(Strlen, returns_five_for_hello) {
	assert_that(strlen("hello"), is_equal_to(5));
}

Describe(BitShiftRight);
BeforeEach(BitShiftRight) {
	int number = 5;
}
AfterEach(BitShiftRight) {}

Ensure(BitShiftRight, shiftsBitRight) {
	int result = bit_shift_right(number);
	assert_that(result, is_equal_to(2));
}

int main(int argc, char** argv) {

	return 0;
}

