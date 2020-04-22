
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

Describe(ControllerCalculateActuation);
BeforeEach(ControllerCalculateActuation) {}
AfterEach(ControllerCalculateActuation) {}

Ensure(ControllerCalculateActuation, test_one) {
    // Setup (heading of zero and a speed of one)
    float delta_heading = 0;
    bool speed = 1;

    float theta_L = 0;
    float theta_R = 0;

    float expected_L_value = 1;
    float expected_R_value = 1;

    float result = ControllerCalculateActuation(delta_heading, speed, &theta_L, &theta_R);

    // Assert that both theta values equal 1
    assert_that(expected_L_value, is_equal_to(theta_L));
    assert_that(expected_R_value, is_equal_to(theta_R));
}

Ensure(ControllerCalculateActuation, test_two) {
    // Setup (heading of zero and a speed of zero to turn on the spot)
    float delta_heading = 0;
    bool speed = 0;

    float theta_L = 0;
    float theta_R = 0;

    float expected_L_value = 1;
    float expected_R_value = -1;

    float result = ControllerCalculateActuation(delta_heading, speed, &theta_L, &theta_R);

    // Assert that theta_L equals 1 and theta_R equals -1
    assert_that(expected_L_value, is_equal_to(theta_L));
    assert_that(expected_R_value, is_equal_to(theta_R));
}