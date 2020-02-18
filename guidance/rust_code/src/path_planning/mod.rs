use crate::error::Error;
use crate::graph::{Graph, TangentialPoint};
use crate::constants::ROBOT_RADIUS;
use std::f64;

fn plan_path() {
    // TODO: Send Message to UI
    

    // TODO: Is robot on edge?
    unimplemented!();

}

pub(self) fn robot_on_edge(graph: &Graph<TangentialPoint>, robot_loc: &TangentialPoint) -> Result<(), Error> {

    let mut min = f64::MAX;

    for edge in graph.edges.iter() {
        let point_n = edge.points.iter();
        let mut point_n_plus_1  = edge.points.iter();
        point_n_plus_1.next();

        if edge.name.contains("entrance") {
            println!("Remove this line");
        }
        
        let steps = 80.0;
        let points_n_n_plus_1 = point_n.zip(point_n_plus_1);
        for (point_n, point_n_plus_1) in points_n_n_plus_1 {
            // segmenting points on the line into more points
            let (start_x, final_x) = (point_n.x, point_n_plus_1.x);
            let (start_y, final_y) = (point_n.y, point_n_plus_1.y);
            let (start_z, final_z) = (point_n.z, point_n_plus_1.z);
            
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