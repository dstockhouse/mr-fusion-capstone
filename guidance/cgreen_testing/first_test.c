#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

Describe(Strlen);
BeforeEach(Strlen) {}
AfterEach(Strlen) {}

Ensure(Strlen, returns_five_for_hello) {
    assert_that(strlen("hello"), is_equal_to(5));
}

int add_one(int number) {
    return number + 1;
}

int main(int argc, char **argv) {
    printf("Hello World!\n");
}