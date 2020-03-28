
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

Describe(ImageProcessing);
BeforeEach(ImageProcessing) {}
AfterEach(ImageProcessing) {}

Ensure(ImageProcessing, dummy_test) {
    assert_that(1, is_equal_to(1));
}

