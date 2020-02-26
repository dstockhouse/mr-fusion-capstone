use std::fs::File;
use std::io::BufReader;
use std::f64;
use std::ops::Sub;

pub mod conversions;

use conversions::{IntoTangential};
use gpx;
use nalgebra::DMatrix;

// Clone trait only for unit testing
#[derive(Debug, Clone, PartialEq)]
pub struct Point {
    pub gps: GPSPointDeg,
    pub tangential: TangentialPoint
}

// Clone Trait only for unit testing
#[derive(Debug, PartialEq, Clone)]
pub struct TangentialPoint {
    pub x: f64,
    pub y: f64,
    pub z: f64
}

impl Sub for &TangentialPoint {
    type Output = (f64, f64, f64);

    fn sub(self, other: Self) -> (f64, f64, f64) {
        (self.x-other.x, self.y-other.y, self.z-other.z)
    }
}

impl TangentialPoint {
    pub fn distance(&self, other: &Self) -> f64 {
        let (x, y, z) = other - self;

        (x.powi(2) + y.powi(2) + z.powi(2)).sqrt()
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

        let poitns_n = points.iter().map(|point| &point.tangential);
        let mut points_n_plus_1 = points.iter().map(|point| &point.tangential);
        points_n_plus_1.next();

        let points_n_n_plus_1 = poitns_n.zip(points_n_plus_1);

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

#[derive(Debug, PartialEq)]
// A is a lifetime
pub struct Vertex {
    // Will be used to display key locations to UI
    pub name: String,

    // Will be used to identify adjacent nodes and edges. T will be either a GPSPoint or a point
    // in our tangential frame.
    pub(self) point: Point,

    // Key data to determine the shortest path using Dijkstra's Algorithm
    pub parent: Option<VertexIndex>,
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
            parent: None,
            tentative_distance: f64::MAX,
            visited: false
        }
    }
}

#[derive(Debug, PartialEq, Clone, Copy)]
pub struct EdgeIndex(pub usize);

#[derive(Debug, PartialEq, Clone, Copy)]
pub struct VertexIndex(pub usize);

// Element at a matrix_index[i][j] indicates an Edge Index
#[derive(Debug, PartialEq)]
pub struct MatrixIndex {
    pub ith: VertexIndex,
    pub jth: VertexIndex,
}

impl MatrixIndex {

    pub fn edge<'a, 'b>(&'a self, graph: &'b Graph) -> &'b Edge {
        let edge_index = graph.connection_matrix[(self.ith.0, self.jth.0)].unwrap();
        &graph.edges[edge_index.0]
    }

    pub fn vertices<'a, 'b>(&'a self, graph: &'b Graph) -> (&'b Vertex, &'b Vertex) {
        let (index_1, index_2) = (self.ith.0, self.jth.0);

        (&graph.vertices[index_1], &graph.vertices[index_2])
    }
}


#[derive(Debug)]
pub struct Graph {
    pub vertices: Vec<Vertex>,
    pub edges: Vec<Edge>,
    pub connection_matrix: DMatrix<Option<EdgeIndex>> // D stands for dynamic
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
    
    for (edege_index, edge) in edges.iter().enumerate() {
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

        let start_vertex_index = start_vertex_index.unwrap();
        let end_vertex_index = end_vertex_index.unwrap();

        connection_matrix[(start_vertex_index, end_vertex_index)] = Some(EdgeIndex(edege_index));
        connection_matrix[(end_vertex_index, start_vertex_index)] = Some(EdgeIndex(edege_index));
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

    return graph

}



#[cfg(test)]
mod tests;