use std::fs::File;
use std::io::BufReader;
use std::f64;
use std::ops::Sub;

pub mod conversions;

use conversions::IntoTangential;
use geojson::{Feature, FeatureCollection, Value, Geometry, feature::Id};
use gpx;

#[derive(Debug)]
pub struct Point {
    pub gps: GPSPointDeg,
    pub tangential: TangentialPoint
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

#[derive(Debug)]
pub struct Edge {
    // For debugging, finding out what line we are looking at on the map
    pub name: String,

    // T, in this case, can be a point relative to our tangential frame, or the gps frame
    pub points: Vec<Point>,
}

#[derive(Debug)]
// A is a lifetime
pub struct Vertex<'a> {
    // Will be used to display key locations to UI
    pub name: String,

    // Will be used to identify adjacent nodes and edges. T will be either a GPSPoint or a point
    // in our tangential frame.
    pub(self) point: Point,

    // Key data to determine the shortest path using Dijkstra's Algorithm
    pub parent_vertex: Option<&'a Vertex<'a>>,
    pub tentative_distance: Option<f64>,
}

impl <'a> Vertex<'a> {
    fn new(name: String, gps: GPSPointDeg) -> Vertex<'a> {

        let tangential = gps.into_tangential();

        let point = Point {
            gps,
            tangential
        };

        Vertex {
            name,
            point,
            parent_vertex: None,
            tentative_distance: None,
        }
    }
}

impl<'a> PartialEq for Vertex<'a>  {
    fn eq(&self, other: &Vertex<'a>) -> bool {

        if self.point.gps != other.point.gps {
            return true;
        }

        false
    }
}

type EdgeIndex = usize;

pub struct Graph <'a> {
    pub vertices: Vec<Vertex<'a>>,
    pub edges: Vec<Edge>,
    pub connection_matrix: Vec<Vec<Option<EdgeIndex>>> //Matrix of indices of edges
}

pub(self) fn connect_vertices_with_edges(
    edges: Vec<Edge>, 
    vertices: Vec<Vertex>
) -> Graph {

    let mut connection_matrix = vec![vec![None; vertices.len()]; vertices.len()];
    
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

        connection_matrix[start_vertex_index][end_vertex_index] = Some(edege_index);
        connection_matrix[end_vertex_index][start_vertex_index] = Some(edege_index);
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
            let gps_points = track.segments[0].points.iter()
                .map(|waypoint| {
                    let height = waypoint.elevation.unwrap();
                    let (long, lat) = waypoint.point().x_y();
                    let gps = GPSPointDeg{long, lat, height};
                    let tangential = gps.into_tangential();

                    Point{gps, tangential}
                })
                .collect::<Vec<Point>>();

            let name = track.name.unwrap();

            Edge{name, points: gps_points}
        })
        .collect::<Vec<Edge>>();
    
    let graph = connect_vertices_with_edges(edges, vertices);

    return graph

}

pub fn graph_to_geo_json_string(graph: &Graph) -> String {
    // Allocating Memory
    let number_of_vertices_and_edges = graph.edges.len() + graph.vertices.len();
    let mut features = Vec::with_capacity(number_of_vertices_and_edges);

    for edge in graph.edges.iter() {
        let edge_points = edge.points.iter()
            .map(|point| vec![point.gps.long, point.gps.lat])
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
            vertex.point.gps.long,
            vertex.point.gps.lat
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