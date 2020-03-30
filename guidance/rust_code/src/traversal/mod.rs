use crate::graph::TangentialPoint;

// Determines when we increment to our next point. Once we have traversed the
// percentage of 1 - OFFSET_FACTOR, we increment the point we are attempting to
// traverse to.
constant OFFSET_FACTOR: f64 = 0.09;

pub struct ProximityLine {
    m: f64,
    b: f64
}

impl ProximityLine {
    pub fn new (
        robot_loc: &TangentialPoint, 
        next_point: &TangentialPoint
    ) {
        
    }
}

fn determine_proximity_line(
    
) {
    let ideal_traversal_vector = next_point - robot_loc;
    unimplemented!()
}