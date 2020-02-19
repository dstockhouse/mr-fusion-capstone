use crate::error::Error;
use crate::graph::{Graph, TangentialPoint};
use crate::constants::ROBOT_RADIUS;
use std::f64;

fn plan_path() {
    // TODO: Send Message to UI
    

    // TODO: Is robot on edge?
    unimplemented!();

}

pub(self) fn robot_on_edge(graph: &Graph, robot_loc: &TangentialPoint) -> Result<(), Error> {

    for edge in graph.edges.iter() {
        
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
                    return Ok(());
                }

                x += x_step;
                y += y_step;
                z += z_step;
            }
        }
    }
    
    Err(Error::RobotNotOnMap)
    
}

#[cfg(test)]
mod tests;