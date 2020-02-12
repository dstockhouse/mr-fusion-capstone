use std::fs::File;
use std::io::{prelude::*, BufReader};
use geojson::{Feature, FeatureCollection, Value, Geometry, feature::Id};
use std::f64;
use std::f64::consts::PI;
use gpx;

#[derive(Debug)]
pub struct GPSPoint {
    pub longitude: f64,
    pub latitude: f64,
    pub height: f64
}

impl PartialEq for GPSPoint {
    fn eq(&self, other: &Self) -> bool{
        self.latitude == other.latitude 
            &&
        self.longitude == other.longitude
    }
}

impl GPSPoint {
    /// Takes two GPS points and determines the distance between them.
    /// Source of algorithm https://www.movable-type.co.uk/scripts/latlong.html
    fn distance(&self, other: &Self) -> f64 {
        let r = 6371e3; // Radious of Earth m
        let lat1 = self.latitude * PI/180.0; // rad
        let long1 = self.longitude * PI/180.0; //rad
        let lat2 = other.longitude * PI/180.0; //rad
        let long2 = other.longitude * PI/180.0; //rad

        let a = ((lat2 - lat1)/2.0).sin().powi(2) + lat1.cos()*lat2.cos() * ((long2 - long1)/2.0).sin().powi(2);
        let c = 2.0 * (a.sqrt().atan2((1.0-a).sqrt()));

        return r * c;
    }
}

#[derive(Debug)]
pub struct Edge {
    // For debugging, finding out what line we are looking at on the map
    pub name: String,
    pub gps_points: Vec<GPSPoint>,
}

#[derive(Debug)]
pub struct Vertex {
    // Will be used to display key locations to UI
    pub name: String,

    // Will be used to identify adjacent nodes and edges
    pub(self) gps_point: GPSPoint,

    // Key data to determine the shortest path using Dijkstra's Algorithm
    pub parent_vertex: Option<&'static Vertex>,
    pub tentative_distance: Option<f64>,
}

pub struct Graph {
    pub vertices: Vec<Vertex>,
    pub edges: Vec<Edge>,
    pub connection_matrix: Vec<Vec<Option<usize>>> //Matrix of indices of edges
}

impl Vertex {
    fn new(name: String, gps_point: GPSPoint) -> Vertex {
        Vertex {
            name,
            gps_point,
            parent_vertex: None,
            tentative_distance: None,
        }
    }
}

impl PartialEq for Vertex {
    fn eq(&self, other: &Vertex) -> bool {
        if self.name == other.name && self.gps_point == other.gps_point {
            return true;
        }

        false
    }
}

pub(self) fn parse_gps_string(gps_string: &String) -> (f64, f64, f64) {
    let gps_data = gps_string
        .trim() // Remove whitespace
        .split(",") // Remove commas and store remaining information in Vec
        .map(|s| s.parse().unwrap()) // Convert vec of strings to floating points
        .collect::<Vec<f64>>();

    let longitude = gps_data[0];
    let latitude = gps_data[1];
    let height = gps_data[2];

    (longitude, latitude, height)
}

pub(self) fn connect_vertices_with_edges(
    edges: Vec<Edge>, 
    vertices: Vec<Vertex>
) -> Graph {

    let connection_matrix = vec![vec![None; vertices.len()]; vertices.len()];
    let mut graph = Graph {
        edges,
        vertices,
        connection_matrix
    };
    
    for edge_number in 0..graph.edges.len() {
        let start_of_edge = graph.edges[edge_number].gps_points.first();
        let end_of_edge = graph.edges[edge_number].gps_points.last();
        let mut start_vertex_index = None;
        let mut end_vertex_index = None;

        for vertex_number in 0..graph.vertices.len() {
            let vertex = &graph.vertices[vertex_number];
            if Some(&vertex.gps_point) == start_of_edge {
                start_vertex_index = Some(vertex_number as usize);
            }
            else if Some(&vertex.gps_point) ==  end_of_edge {
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


pub fn initialize_from_kml_file(name: &str) -> Graph {
    // Open file and read contents to memory
    let file = File::open(name).unwrap();
    let reader = BufReader::new(file);

    let gpx_data = gpx::read(reader).unwrap();

    let vertices = gpx_data.waypoints.into_iter()
        .map(|vertex_data| {
            // long, lat order is intentional. They are stored in this order in the file.
            let (longitude, latitude) = vertex_data.point().x_y();

            let height = vertex_data.elevation.unwrap();
            let name = vertex_data.name.unwrap();
            
            
            Vertex::new(name, GPSPoint{longitude, latitude, height})
        })
        .collect::<Vec<Vertex>>();

    let edges = gpx_data.tracks.into_iter() 
        .map(|track| {
            // Indexing at 0 since for every track we are guaranteed to only have move segment.
            let gps_points = track.segments[0].points.iter()
                .map(|waypoint| {
                    let height = waypoint.elevation.unwrap();
                    let (longitude, latitude) = waypoint.point().x_y();

                    GPSPoint{longitude, latitude, height}
                })
                .collect::<Vec<GPSPoint>>();

            let name = track.name.unwrap();

            Edge{name, gps_points}
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
        let edge_points = edge.gps_points.iter()
            .map(|point| vec![point.longitude, point.latitude])
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
            vertex.gps_point.longitude,
            vertex.gps_point.latitude
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