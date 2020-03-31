
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

Describe(VN200);
BeforeEach(VN200) {}
AfterEach(VN200) {}

Ensure(VN200, dummy_test) {
    assert_that(1, is_equal_to(1));
}

