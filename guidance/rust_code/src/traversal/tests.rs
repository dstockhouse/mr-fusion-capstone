use super::*;

// TO confirm, use the geogebra tool and put the robot_loc and next point in the
// locations corresponding to the test.
//
// https://www.geogebra.org/classic/jwwnnxy6

#[test]
fn new_proximity_line() {
    let prev_point = TangentialPoint::new(0.0, 1.2, 0.0);
    let next_point = TangentialPoint::new(2.0, 1.0, 0.0);

    let proximity_line = ProximityLine::new(&prev_point, &next_point);

    assert!(
        proximity_line.m > 9.9
            && proximity_line.m < 10.1
            && proximity_line.b > -17.19
            && proximity_line.b < -17.17
    );
}

#[test]
fn horizontal_proximity_line() {
    let prev_point = TangentialPoint::new(0.0, 1.0, 0.0);
    let next_point = TangentialPoint::new(0.0, 3.0, 0.0);

    let proximity_line = ProximityLine::new(&prev_point, &next_point);

    assert_eq!(proximity_line.m, 0.0);
}

#[test]
fn vertical_proximity_line() {
    use std::f64;

    let prev_point = TangentialPoint::new(0.0, 1.0, 0.0);
    let next_point = TangentialPoint::new(2.0, 1.0, 0.0);

    let proximity_line = ProximityLine::new(&prev_point, &next_point);

    assert_eq!(proximity_line.m, f64::NEG_INFINITY);
}
