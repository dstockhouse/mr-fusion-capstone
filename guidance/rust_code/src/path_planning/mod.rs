use crate::graph::{Graph, TangentialPoint, MatrixIndex, EdgeIndex, VertexIndex};
use crate::constants::ROBOT_RADIUS;
use crate::error::Error;
use crate::States;



fn plan_path() -> Result<States, Error> {
    // TODO: Send Message to UI
    

    // TODO: Is robot on edge?
    unimplemented!();

}

type Path = Vec<MatrixIndex>;

impl<'a> Graph<'a> {
    /// Returns an error if the robot is not on the edge. Otherwise,
    /// returns the Matrix index indicating the edge containing the robot.
    fn closest_edge_to(&self, robot_loc: &TangentialPoint) -> Result<MatrixIndex, Error> {

        let edges_and_indices = self.edges.iter()
            .enumerate()
            .map(|(index, edge)| 
                (EdgeIndex(index), edge)
            );

        for (edge_index, edge) in edges_and_indices {

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
                        return self.connection_matrix_index_from(edge_index);
                    }

                    x += x_step;
                    y += y_step;
                    z += z_step;
                }
            }
        }
        
        Err(Error::PathPlanningNotOnMap)
    }

    fn connection_matrix_index_from(&self, edge_index: EdgeIndex) -> Result<MatrixIndex, Error> {

        for ((row,col), edge) in self.connection_matrix.iter().enumerate() {
            for (column_index, edge_index_in_connection_matrix) in row.iter().enumerate() {
                if Some(edge_index) == *edge_index_in_connection_matrix {
                    return Ok(MatrixIndex{ 
                        ith: VertexIndex(row_index), 
                        jth: VertexIndex(column_index)
                    })
                }
            }
        }

        Err(Error::PathPlanningEdgeIndexNotInConnectionMatrix)
    }
    
    fn shortest_path(&self, start: VertexIndex, end: VertexIndex) -> Result<Path, Error> {
        unimplemented!()
    }

}

#[cfg(test)]
mod tests;