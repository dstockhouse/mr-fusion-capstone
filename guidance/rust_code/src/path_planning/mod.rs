use std::f64;

use crate::graph::{Graph, TangentialPoint, MatrixIndex, EdgeIndex, VertexIndex, Vertex};
use crate::constants::ROBOT_RADIUS;
use crate::error::Error;
use crate::States;



fn plan_path() -> Result<States, Error> {
    // TODO: Send Message to UI
    

    // TODO: Is robot on edge?
    unimplemented!();

}

type Path = Vec<MatrixIndex>;

impl Graph {
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

        for row in 0..self.connection_matrix.nrows() {
            for col in 0..self.connection_matrix.ncols() {
                if Some(edge_index) == self.connection_matrix[(row, col)] {
                    return Ok(MatrixIndex{ 
                        ith: VertexIndex(row), 
                        jth: VertexIndex(col)
                    })
                }
            }
        }

        Err(Error::PathPlanningEdgeIndexNotInConnectionMatrix)
    }
    
    fn shortest_path(&mut self, start: VertexIndex, end: VertexIndex) -> Result<Path, Error> {

        // Initializing the the graph so all tentative distances are 0
        // and not parent vertices are set.
        for vertex in self.vertices.iter_mut() {
            vertex.tentative_distance = f64::MAX;
            vertex.visited = false;
            vertex.parent = None;
        }

        // Setting the tentative distance of the start vertex to 0
        self.vertices[start.0].tentative_distance = 0.0;

        let mut vertex_index = start; // Keeps track of currently visiting vertex
        let mut nodes_not_visited = self.vertices.len();

        while nodes_not_visited != 0 && vertex_index != end {
            let mut vertex = &mut self.vertices[vertex_index.0] as *mut Vertex;
            let connecting_edges = self.connection_matrix.row(vertex_index.0);
            let adj_vertices = connecting_edges.iter()
                .enumerate()
                .filter(|(_, edge_index)| edge_index.is_some())
                .map(|(vertex_index, edge_index)| (vertex_index, edge_index.unwrap()));

            for (adj_vertex_index, edge_index) in adj_vertices {
                let adj_vertex = &mut self.vertices[adj_vertex_index] as *mut Vertex;
                let connecting_edge = &self.edges[edge_index.0];

                // Using the unsafe keywords since more than one mutable reference is needed.
                // One to the currently visiting vertex and the other to the adjacent vertex.
                unsafe {
                    if !(*adj_vertex).visited {
                        // Using the unsafe keywords since more than one mutable reference is needed.
                        // One to the currently visiting vertex and the other to the adjacent vertex.
                        let temp_distance = 
                        (*vertex).tentative_distance + connecting_edge.distance;

                        if temp_distance < (*adj_vertex).tentative_distance {
                            (*adj_vertex).tentative_distance = temp_distance;
                            (*adj_vertex).parent = Some(vertex_index);
                        }
                    }
                }
            }
            unsafe {(*vertex).visited = true;}
            nodes_not_visited -= 1;

            let mut min_dist = f64::MAX;
            for (index, vertex) in self.vertices.iter().enumerate() {
                if vertex.tentative_distance < min_dist && !vertex.visited {
                    min_dist = vertex.tentative_distance;
                    vertex_index = VertexIndex(index);
                }
            }

        }
        
        

        unimplemented!();
    }

}

#[cfg(test)]
mod tests;