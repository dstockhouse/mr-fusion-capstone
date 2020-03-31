// The following is a link to our proof of concept
//
// https://www.geogebra.org/classic/jwwnnxy6


use crate::graph::TangentialPoint;

// Determines when we increment to our next point. Once we have traversed the
// percentage of 1 - OFFSET_FACTOR, we increment the point we are attempting to
// traverse to.
const OFFSET_FACTOR: f64 = 0.09;

pub struct ProximityLine {
    m: f64,
    b: f64
}

impl ProximityLine {
    // Create the proximity line given the robot location and the next point we 
    // wish to traverse to. Be sure to consult the proof of concept to get a
    // visual understanding for what this function is doing.
    //
    // https://www.geogebra.org/classic/jwwnnxy6
    pub fn new (
        robot_loc: &TangentialPoint, 
        next_point: &TangentialPoint
    ) -> Self {
        // The vector that goes from our robot to the next point to traverse.
        let ideal_traversal_v = next_point - robot_loc;

        // The amount we move back from next point to place our proximity line.
        let proximity_offset = ideal_traversal_v * OFFSET_FACTOR; 

        // A point that will lie on the proximity line
        let proximity_point = next_point - &proximity_offset;

        // The slope of the proximity line. Garunteed to be perpendicular to the 
        // line that goes from our robot to the next desired point
        let m = (-1.0)*ideal_traversal_v.y() / ideal_traversal_v.x();

        // The y intercept of our line
        let b = proximity_point.y() - m*proximity_point.x();

        ProximityLine {m, b}
            
        
    }
}

#[cfg(test)]
mod tests;