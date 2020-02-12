use crate::error::Error;
use crate::graph::{Graph, GPSPoint};
use crate::constants::ROBOT_RADIOUS;
use std::f64;

fn plan_path() {
    // TODO: Send Message to UI
    unimplemented!();

    // TODO: Is robot on edge?
    unimplemented!();

}

pub(self) fn robot_on_edge(graph: &Graph, robot_loc: &GPSPoint) -> Result<(), Error> {

    let mut min = f64::MAX;
    
    let edges = &graph.edges;

    for edge in edges.iter() {
        let gps_point_n = edge.gps_points.iter();
        let mut gps_point_n_plus_1  = edge.gps_points.iter();
        gps_point_n_plus_1.next();

        let points_n_n_plus_1 = gps_point_n.zip(gps_point_n_plus_1);

        for (gps_point_n, gps_point_n_plus_1) in points_n_n_plus_1 {
            // segmenting points on the line into more points
            let steps = 80.0;
            let start_x = gps_point_n.latitude;
            let final_x = gps_point_n_plus_1.latitude;
            let start_y = gps_point_n.longitude;
            let final_y = gps_point_n_plus_1.longitude;
            
            let x_step = (final_x - start_x) / steps;
            let y_step = (final_y - start_y) / steps;

            let mut x = start_x;
            let mut y = start_y;
            for _ in 0..steps as u32 {
                x = x + x_step;
                y = y + y_step;

                let x_diff =  x - robot_loc.latitude;
                let y_diff = y - robot_loc.longitude;
                if min > (x - robot_loc.latitude).powi(2) + (y - robot_loc.longitude).powi(2) {
                    min = (x - robot_loc.latitude).powi(2) + (y - robot_loc.longitude).powi(2)
                }
                
                if (x - robot_loc.latitude).powi(2) + (y - robot_loc.longitude).powi(2) <= ROBOT_RADIOUS.powi(2) {
                    // Then the robot's radius contains an edge
                    return Ok(());
                }
            }
        }
    }
    
    println!("min: {}", min);
    Err(Error::RobotNotOnMap)
    
}

#[cfg(test)]
mod tests;