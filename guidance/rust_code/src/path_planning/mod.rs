use std::f64;

use crate::graph::*;
use crate::constants::ROBOT_RADIUS;
use crate::error::Error;
use crate::States;

fn plan_path() -> Result<States, Error> {
    // TODO: Send Message to UI
    

    // TODO: Is robot on edge?
    unimplemented!();

}

#[derive(Debug, PartialEq)]
pub struct Path {
    indices: Vec<MatrixIndex>,
}

impl Edges for Path {
    fn edges<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Edge> {
        self.indices.iter()
        .map(|matrix_index| matrix_index.edge(graph))
        .collect()
    }
}

impl Edges for &Path {
    fn edges<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Edge> {
        self.indices.iter()
        .map(|matrix_index| matrix_index.edge(graph))
        .collect()
    }
}

impl Vertices for Path {

    fn vertices<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Vertex> {
        // Allocating Memory
        let mut vertices = Vec::with_capacity(2 * self.indices.len());

        let vertices_start_end = self.indices.iter()
            .map(|matrix_index| matrix_index.vertices(graph));

        for (vertex_1, vertex_2) in vertices_start_end {
            vertices.push(vertex_1);
            vertices.push(vertex_2);
        }

        // Return the vertex with repeated elements removed
        vertices.dedup();

        vertices

    }
}

impl Vertices for &Path {

    fn vertices<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Vertex> {
        // Allocating Memory
        let mut vertices = Vec::with_capacity(2 * self.indices.len());

        let vertices_start_end = self.indices.iter()
            .map(|matrix_index| matrix_index.vertices(graph));

        for (vertex_1, vertex_2) in vertices_start_end {
            vertices.push(vertex_1);
            vertices.push(vertex_2);
        }

        // Return the vertex with repeated elements removed
        vertices.dedup();

        vertices

    }
}

impl Graph {
    /// Returns an error if the robot is not on the edge. Otherwise,
    /// returns the Matrix index indicating the edge containing the robot.
    fn closest_edge_to(&self, robot_loc: &TangentialPoint) -> Result<MatrixIndex, Error> {

        let edges_and_indices = self.edges.iter()
            .enumerate()
            .map(|(index, edge)| 
                (EdgeIndex(index), edge)
            );
        
        let mut closest_point = TangentialPoint::new(f64::MAX, f64::MAX, f64::MAX);
        let mut closest_edge_index = None;

        for (edge_index, edge) in edges_and_indices {

            let point_n = edge.points.iter();
            let mut point_n_plus_1  = edge.points.iter();
            point_n_plus_1.next();
            
            let steps = 80.0;
            let points_n_n_plus_1 = point_n.zip(point_n_plus_1);
            for (point_n, point_n_plus_1) in points_n_n_plus_1 {
                // segmenting points on the line into more points
                let (start_x, final_x) = (point_n.tangential.x(), point_n_plus_1.tangential.x());
                let (start_y, final_y) = (point_n.tangential.y(), point_n_plus_1.tangential.y());
                let (start_z, final_z) = (point_n.tangential.z(), point_n_plus_1.tangential.z());
                
                let (x_step, y_step, z_step) = ((final_x - start_x) / steps, 
                                                (final_y - start_y) / steps,
                                                (final_z - start_z) / steps);

                let (mut x, mut y, mut z) = (start_x, start_y, start_z);

                for _ in 0..steps as u32 {
                    
                    let temp_point = TangentialPoint::new(x, y, z);
                    let temp_distance_to_robot = temp_point.distance(robot_loc);
                    
                    if temp_distance_to_robot <= ROBOT_RADIUS && 
                    temp_distance_to_robot < closest_point.distance(robot_loc) {
                        closest_point = temp_point;
                        closest_edge_index = Some(edge_index);
                    }

                    x += x_step;
                    y += y_step;
                    z += z_step;
                }
            }
        }
        
        match closest_edge_index {
            None => return Err(Error::PathPlanningNotOnMap),
            Some(closest_edge_index) => self.connection_matrix_index_from(closest_edge_index)
        }
    }

    fn connection_matrix_index_from(&self, edge_index: EdgeIndex) -> Result<MatrixIndex, Error> {

        for row in 0..self.connection_matrix.nrows() {
            for col in 0..self.connection_matrix.ncols() {
                if Some(edge_index) == self.connection_matrix[(row, col)] {
                    return Ok(MatrixIndex{ 
                        ith: VertexIndex(row), 
                        jth: VertexIndex(col),
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

            // Selecting the next node to visit
            let mut min_dist = f64::MAX;
            for (index, vertex) in self.vertices.iter().enumerate() {
                if vertex.tentative_distance < min_dist && !vertex.visited {
                    min_dist = vertex.tentative_distance;
                    vertex_index = VertexIndex(index);
                }
            }
        }

        // After completing the while loop, we have either found the shortest path,
        // or one does not exist. If one exists, then a stack of matrix indices will be outputted
        // otherwise, an error message will be returned, indicating the path does not exist.
        let dest_vertex = &self.vertices[end.0];
        if dest_vertex.tentative_distance == f64::MAX {
            return Err(Error::PathPlanningPathDoesNotExist);
        }
        
        // Pre allocating memory. Chosen path will never exceed the number of edges in the graph.
        let mut connection_matrix_indices = Vec::with_capacity(self.edges.len());

        let mut path_vertex_index = end;
        while path_vertex_index != start {

            let parent_vertex_index = self.vertices[path_vertex_index.0].parent.unwrap();
            let matrix_index = MatrixIndex {
                ith: parent_vertex_index,
                jth: path_vertex_index
            };

            connection_matrix_indices.push(matrix_index);

            path_vertex_index = parent_vertex_index;

        }

        // Reversing the order of the connection matrix index so instructions are given from the
        // start to the end and not end to start.
        connection_matrix_indices = connection_matrix_indices.into_iter().rev().collect();

        Ok(Path{indices: connection_matrix_indices})
    }

}

#[cfg(test)]
mod tests;