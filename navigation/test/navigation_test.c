
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

#include "nav_frames.h"

Describe(Navigation);
BeforeEach(Navigation) {}
AfterEach(Navigation) {}

Ensure(Navigation, dummy_test) {

    // Follow similar form for your assertions
    // assert_that(this, is_equal_to(that));

}

Describe(llh_to_xyz);
BeforeEach(llh_to_xyz) {}
AfterEach(llh_to_xyz) {}

Ensure(llh_to_xyz, returns_origin_coordiantes) {
    // Test Values 
    double llh[3] = [LAT_ORIGIN, LONG_ORIGIN, HEIGHT_ORIGIN];
    double xyz[3];
   
    // Values from matlab run
    double ML_xyz[3];
    ML_xyz[1] = -2.007273864110492 * pow(10, 6); // Matlab X coord
    ML_xyz[2] = -4.857743263361342 * pow(10, 6); // Matlab Y coord
    ML_xyz[3] =  3.603678639618603 * pow(10, 6); // Matlab Z coord

    // Testing
    significant_figures_for_assert_double_are(3)
    llh_to_xyz(&llh, &xyz)
    assert_that_double(xyz, is_equal_to_double(ML_xyz);
}
