# What is a Unit Test?

A function that tests that another function is working properly.

Ex.
```c
#include <cgreen/cgreen.h>

Describe(Strlen);
BeforeEach(Strlen) {}
AfterEach(Strlen) {}

Ensure(Strlen, returns_five_for_hello) {
	assert_that(strlen("hello"), is_equal_to(5));
}
```

Here the Unit test is checking if `strlen` says there are 5 characters in the string "hello".

# Characteristics of a Good Test
In general, unit tests always have the following

1. Some setup. Making sure that you have all the data you need before you call a function.
2. A call to the function that you wish to test.
3. An assertion that the function is returning something that you expect.
4. Complete independance from other tests failing or passing.

A really good talk describing these basics can be found [here](https://youtu.be/fr1E9aVnBxw?t=116).

# Template
Our unit tests are mostly going to follow this form.

```c
#include <cgreen/cgreen.h>

Describe(FunctionUnderTest);
BeforeEach(FunctionUnderTest) {}
AfterEach(FunctionUnderTest) {}

Ensure(FunctionUnderTest, expected_behavior_description) {
    // Setup

    // Use a setup function when multiple unit tests share the same setup.
    int var1 = SetupFunction();
    int var2 = 2;
    char var3 = 'h';

    int expectedResult = 22;

    // Function call of our application
	int result = functionUnderTest(var1, var2, var3);

    // The assertion that we got what we expected
    assert_that(expected_result, is_equal_to(result));
}
```

# What to Avoid During Unit Tests
Function that require network connections or Sensor output. We can get around this by something called mocking. Come fine me if you think you need to do this.

# Exercises
See `navigation_test.c`