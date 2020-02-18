use std::fs::File;
use std::io::BufReader;
use std::f64;
use std::ops::Sub;

pub mod conversions;

use geojson::{Feature, FeatureCollection, Value, Geometry, feature::Id};
use gpx;

pub trait Point {
    fn get(&self) -> (f64, f64, f64);
}

#[derive(Debug, PartialEq)]
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

#[derive(Debug)]
pub struct GPSPoint {
    pub lat: f64,
    pub long: f64,
    pub height: f64
}

impl Point for GPSPoint {
    fn get(&self) -> (f64, f64, f64) {
        (self.lat, self.long, self.height)
    }
}

impl PartialEq for GPSPoint {
    fn eq(&self, other: &Self) -> bool {
        self.lat == other.lat 
            &&
        self.long == other.long
    }
}

#[derive(Debug)]
pub struct Edge<T> {
    // For debugging, finding out what line we are looking at on the map
    pub name: String,

    // T, in this case, can be a point relative to our tangential frame, or the gps frame
    pub points: Vec<T>,
}

#[derive(Debug)]
// A is a lifetime
pub struct Vertex<'a, T> {
    // Will be used to display key locations to UI
    pub name: String,

    // Will be used to identify adjacent nodes and edges. T will be either a GPSPoint or a point
    // in our tangential frame.
    pub(self) point: T,

    // Key data to determine the shortest path using Dijkstra's Algorithm
    pub parent_vertex: Option<&'a Vertex<'a, T>>,
    pub tentative_distance: Option<f64>,
}

pub struct Graph <'a, T> {
    pub vertices: Vec<Vertex<'a, T>>,
    pub edges: Vec<Edge<T>>,
    pub connection_matrix: Vec<Vec<Option<usize>>> //Matrix of indices of edges
}

impl <'a, T> Vertex<'a, T> {
    fn new(name: String, point: T) -> Vertex<'a, T> {
        Vertex {
            name,
            point,
            parent_vertex: None,
            tentative_distance: None,
        }
    }
}

impl<'a, T> PartialEq for Vertex<'a, T> 
    where T: Point {
    fn eq(&self, other: &Vertex<'a, T>) -> bool {

        if self.point.get() != other.point.get() {
            return true;
        }

        false
    }
}

pub(self) fn connect_vertices_with_edges(
    edges: Vec<Edge<GPSPoint>>, 
    vertices: Vec<Vertex<GPSPoint>>
) -> Graph<GPSPoint> {

    let connection_matrix = vec![vec![None; vertices.len()]; vertices.len()];
    let mut graph = Graph {
        edges,
        vertices,
        connection_matrix
    };
    
    for edge_number in 0..graph.edges.len() {
        let start_of_edge = &graph.edges[edge_number].points.first().unwrap();
        let end_of_edge = &graph.edges[edge_number].points.last().unwrap();
        let mut start_vertex_index = None;
        let mut end_vertex_index = None;

        for vertex_number in 0..graph.vertices.len() {
            let vertex = &graph.vertices[vertex_number];
            if &&vertex.point == start_of_edge {
                start_vertex_index = Some(vertex_number as usize);
            }
            else if &&vertex.point ==  end_of_edge {
                end_vertex_index = Some(vertex_number as usize);
            }
        }

        let start_vertex_index = start_vertex_index.unwrap();
        let end_vertex_index = end_vertex_index.unwrap();

        graph.connection_matrix[start_vertex_index][end_vertex_index] = Some(edge_number);
        graph.connection_matrix[end_vertex_index][start_vertex_index] = Some(edge_number);
    }

    return graph
}


pub fn initialize_from_gpx_file(name: &str) -> Graph<GPSPoint> {
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
            
            
            Vertex::new(name, GPSPoint{long, lat, height})
        })
        .collect::<Vec<Vertex<GPSPoint>>>();

    let edges = gpx_data.tracks.into_iter() 
        .map(|track| {
            // Indexing at 0 since for every track we are guaranteed to only have move segment.
            let gps_points = track.segments[0].points.iter()
                .map(|waypoint| {
                    let height = waypoint.elevation.unwrap();
                    let (long, lat) = waypoint.point().x_y();

                    GPSPoint{long, lat, height}
                })
                .collect::<Vec<GPSPoint>>();

            let name = track.name.unwrap();

            Edge{name, points: gps_points}
        })
        .collect::<Vec<Edge<GPSPoint>>>();
    
    let graph = connect_vertices_with_edges(edges, vertices);

    return graph

}

pub fn graph_to_geo_json_string(graph: &Graph<GPSPoint>) -> String {
    // Allocating Memory
    let number_of_vertices_and_edges = graph.edges.len() + graph.vertices.len();
    let mut features = Vec::with_capacity(number_of_vertices_and_edges);

    for edge in graph.edges.iter() {
        let edge_points = edge.points.iter()
            .map(|point| vec![point.long, point.lat])
            .collect();

        let geometry = Geometry::new(
            Value::LineString(edge_points)
        );

        let feature = Feature {
            bbox: None,
            geometry: Some(geometry),
            id: Some(Id::String(edge.name.clone())),
            properties: None,
            foreign_members: None
        };
        
        features.push(feature);
    }

    for vertex in graph.vertices.iter() {
        let vertex_point = vec![
            vertex.point.long,
            vertex.point.lat
        ];

        let geometry = Geometry::new(
            Value::Point(vertex_point)
        );

        let feature = Feature {
            bbox: None,
            geometry: Some(geometry),
            id: Some(Id::String(vertex.name.clone())),
            properties: None,
            foreign_members:None
        };

        features.push(feature);
    }

    let feature_collection = FeatureCollection {
        bbox: None,
        features,
        foreign_members: None
    };

    feature_collection.to_string()
}

#[cfg(test)]
mod tests;