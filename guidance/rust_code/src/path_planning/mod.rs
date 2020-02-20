use crate::error::Error;
use crate::graph::{Graph, TangentialPoint, MatrixIndex};
use crate::constants::ROBOT_RADIUS;
use std::f64;

fn plan_path() {
    // TODO: Send Message to UI
    

    // TODO: Is robot on edge?
    unimplemented!();

}


impl<'a> Graph<'a> {
    /// Returns an error if the robot is not on the edge. Otherwise,
    /// returns the Matrix index indicating the edge containing the robot.
    fn closest_edge_to(&self, robot_loc: &TangentialPoint) -> Result<(), Error> {

        for (edge_index, edge) in self.edges.iter().enumerate() {
            
            let point_n = edge.points.iter();
            let mut point_n_plus_1  = edge.points.iter();
            point_n_plus_1.next();
            
            let steps = 80.0;
            let points_n_n_plus_1 = point_n.zip(point_n_plus_1);
            for (point_n, point_n_plus_1) in points_n_n_plus_1 {
                // segmenting points on the line into more points
                let (start_x, final_x) = (point_n.tangential.x, point_n_plus_1.tangential.x);
                let (start_y, final_y) = (point_n.tangential.y, point_n_plus_1.tangential.y);
                let (start_z, final_z) = (point_n.tangential.z, point_n_plus_1.tangential.z);
                
                let (x_step, y_step, z_step) = ((final_x - start_x) / steps, 
                                                (final_y - start_y) / steps,
                                                (final_z - start_z) / steps);

                let (mut x, mut y, mut z) = (start_x, start_y, start_z);

                for _ in 0..steps as u32 {
                    
                    let temp_point = TangentialPoint{x, y, z};
                    
                    if temp_point.distance(robot_loc) <= ROBOT_RADIUS {
                        // TODO: return matrix index here
                    }

                    x += x_step;
                    y += y_step;
                    z += z_step;
                }
            }
        }
        
        Err(Error::RobotNotOnMap)
    }
}


#[cfg(test)]
mod tests;