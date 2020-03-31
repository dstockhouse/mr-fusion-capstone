use super::*;

// TO confirm, use the geogebra tool and put the robot_loc and next point in the 
// locations corresponding to the test.
//
// https://www.geogebra.org/classic/jwwnnxy6

#[test]
fn new_proximity_line() {
    let robot_loc = TangentialPoint::new(0.0, 1.2, 0.0);
    let next_point = TangentialPoint::new(2.0, 1.0, 0.0);
    
    let proximity_line = ProximityLine::new(&robot_loc, &next_point);

    assert!(
        proximity_line.m > 9.9 &&
        proximity_line.m < 10.1 &&

        proximity_line.b > -17.19 &&
        proximity_line.b < -17.17
    );
}

