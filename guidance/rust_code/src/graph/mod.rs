use std::fs::File;
use std::io::BufReader;
use std::f64;
use std::ops::{Sub, Mul};

pub mod conversions;

use conversions::{IntoTangential};
use gpx;
use nalgebra::{DMatrix, Vector3};

// Clone trait only for unit testing
#[derive(Debug, Clone, PartialEq)]
pub struct Point {
    pub gps: GPSPointDeg,
    pub tangential: TangentialPoint
}

// Clone Trait only for unit testing
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct TangentialPoint {
    vector: Vector3<f64>,
}

impl Sub for &TangentialPoint {
    type Output = TangentialPoint;

    fn sub(self, other: Self) -> TangentialPoint {
        TangentialPoint {
            vector: self.vector - other.vector
        }
    }
}

impl Mul<f64> for TangentialPoint {
    type Output = Self;

    fn mul(self, rhs: f64) -> Self::Output {
        TangentialPoint {
            vector: self.vector * rhs
        }
    }
}

impl TangentialPoint {
    pub fn new(x: f64, y: f64, z: f64) -> Self {
        TangentialPoint {
            vector: Vector3::new(x, y, z)
        }
    }
    pub fn x(&self) -> f64 {
        self.vector[0]
    }
    pub fn y(&self) -> f64 {
        self.vector[1]
    }
    pub fn z(&self) -> f64 {
        self.vector[2]
    }
    pub fn distance(&self, other: &Self) -> f64 {
        let point = other - self;

        (point.x().powi(2) + point.y().powi(2) + point.z().powi(2)).sqrt()
    }
}

// Clone trait only for unit testing
#[derive(Debug, Clone)]
pub struct GPSPointDeg {
    pub lat: f64,
    pub long: f64,
    pub height: f64
}

impl PartialEq for GPSPointDeg {
    fn eq(&self, other: &Self) -> bool {
        self.lat == other.lat 
            &&
        self.long == other.long
    }
}

pub(self) struct GPSPointRad {
    pub(self) lat: f64,
    pub(self) long: f64,
    pub(self) height: f64
}

#[derive(Debug, PartialEq)]
pub struct Edge {
    // For debugging, finding out what line we are looking at on the map
    pub name: String,

    // T, in this case, can be a point relative to our tangential frame, or the gps frame
    pub points: Vec<Point>,

    pub distance: f64,
}

impl Edge {
    fn new(name: String, points: Vec<Point>) -> Self {

        // Determining the distance of the edge for construction
        let mut distance = 0.0;

        let points_n = points.iter().map(|point| &point.tangential);
        let mut points_n_plus_1 = points.iter().map(|point| &point.tangential);
        points_n_plus_1.next();

        // Combines the iterator of point_n and point_n_plus_one iterator into a single iterator so if may be consumed
        // in the for loop.
        let points_n_n_plus_1 = points_n.zip(points_n_plus_1);

        for (point_n, points_n_plus_1) in points_n_n_plus_1 {
            distance += point_n.distance(&points_n_plus_1);
        }

        Edge {
            name,
            points,
            distance
        }

    }
}

pub trait Edges {
    fn edges<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Edge>;
}

#[derive(Debug, PartialEq)]
// A is a lifetime
pub struct Vertex {
    // Will be used to display key locations to UI
    pub name: String,

    // Will be used to identify adjacent nodes and edges. T will be either a GPSPoint or a point
    // in our tangential frame.
    pub(self) point: Point,

    // Key data to determine the shortest path using Dijkstra's Algorithm
    pub parent_vertex_index: Option<usize>,
    pub tentative_distance: f64,
    pub visited: bool
}

impl Vertex {
    fn new(name: String, gps: GPSPointDeg) -> Vertex {

        let tangential = gps.into_tangential();

        let point = Point {
            gps,
            tangential
        };

        Vertex {
            name,
            point,
            parent_vertex_index: None,
            tentative_distance: f64::MAX,
            visited: false
        }
    }
}

// Element at a matrix_index[i][j] indicates an Edge Index
#[derive(Debug, PartialEq)]
pub struct MatrixIndex {
    pub ith: usize,
    pub jth: usize,
}

impl MatrixIndex {

    pub fn edge<'a, 'b>(&'a self, graph: &'b Graph) -> &'b Edge {
        let edge_index = graph.connection_matrix[(self.ith, self.jth)].unwrap();
        &graph.edges[edge_index]
    }

    pub fn vertices<'a, 'b>(&'a self, graph: &'b Graph) -> (&'b Vertex, &'b Vertex) {
        let (index_1, index_2) = (self.ith, self.jth);

        (&graph.vertices[index_1], &graph.vertices[index_2])
    }
}

pub trait Vertices {
    fn vertices<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Vertex>;
}

pub struct Graph {
    pub vertices: Vec<Vertex>,
    pub edges: Vec<Edge>,
    pub connection_matrix: DMatrix<Option<usize>> // usize is indicies to the edges
}

impl Vertices for &Graph {
    fn vertices<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Vertex> {
        graph.vertices.iter().collect()
    }
}

impl Edges for &Graph {
    fn edges<'a, 'b>(&'a self, graph: &'b Graph) -> Vec<&'b Edge> {
        graph.edges.iter().collect()
    }
}

pub(self) fn connect_vertices_with_edges(
    edges: Vec<Edge>, 
    vertices: Vec<Vertex>
) -> Graph {

    let vertices_len = vertices.len();

    let mut connection_matrix = DMatrix::from_vec(
        vertices_len, // number of rows
        vertices_len, // number of columns
        vec![None; vertices_len * vertices_len] // initializing the array to None
    );
    
    // Iterating though all edges and vertices. When the first and last point of an edge match two vertices, then the 
    // connection matrix at v_m v_n and v_n v_m gets populated with the index to the edges vector.
    for (edge_index, edge) in edges.iter().enumerate() {
        let start_of_edge = edge.points.first().unwrap();
        let end_of_edge = edge.points.last().unwrap();
        let mut start_vertex_index = None;
        let mut end_vertex_index = None;

        for (vertex_index, vertex) in vertices.iter().enumerate() {
            if vertex.point.gps == start_of_edge.gps {
                start_vertex_index = Some(vertex_index);
            }
            else if vertex.point.gps ==  end_of_edge.gps {
                end_vertex_index = Some(vertex_index);
            }
        }
        
        match (start_vertex_index, end_vertex_index) {
            // If the start and end vertices have been assigned then we know the edge that connects those two vertices.
            // The connection matrix should be updated to reflex that we know the edge and is associated vertices.
            (Some(start_vertex_index), Some(end_vertex_index)) => {
                connection_matrix[(start_vertex_index, end_vertex_index)] = Some(edge_index);
                connection_matrix[(end_vertex_index, start_vertex_index)] = Some(edge_index);
            },
            
            // If either one of them didn't get assigned, panic with some helpful information.
            _ => panic!("{} is dangling in the gpx file", edge.name)
        };

        
    }

    Graph {
        edges,
        vertices,
        connection_matrix
    }
}

pub fn initialize_from_gpx_file(name: &str) -> Graph {
    // Open file and read contents to memory
    let file = File::open(name).unwrap();
    let reader = BufReader::new(file);

    let gpx_data = gpx::read(reader).unwrap();

    let vertices = gpx_data.waypoints.into_iter()
        .map(|vertex_data| {
            // long, lat order is intentional. They are stored in this order in the file.
            let (long, lat) = vertex_data.point().x_y();

            let height = vertex_data.elevation.unwrap();
            let name = vertex_data.name.unwrap();
            
            Vertex::new(name, GPSPointDeg{long, lat, height})
        })
        .collect::<Vec<Vertex>>();

    let edges = gpx_data.tracks.into_iter() 
        .map(|track| {
            // Indexing at 0 since for every track we are guaranteed to only have move segment.
            let points = track.segments[0].points.iter()
                .map(|waypoint| {
                    let height = waypoint.elevation.unwrap();
                    let (long, lat) = waypoint.point().x_y();
                    let gps = GPSPointDeg{long, lat, height};
                    let tangential = gps.into_tangential();

                    Point{gps, tangential}
                })
                .collect::<Vec<Point>>();

            let name = track.name.unwrap();

            Edge::new(name, points)
        })
        .collect::<Vec<Edge>>();
    
    let graph = connect_vertices_with_edges(edges, vertices);

    graph

}



#[cfg(test)]
mod tests;