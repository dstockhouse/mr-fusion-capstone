use std::fs::File;
use std::io::{prelude::*, BufReader};
mod tests;

struct GPSPoint {
    latitude: u32,
    longitude: u32,
}

struct Edge {
    // For debugging, finding out what line we are looking at on the map
    map_name: String,
    gps_points: Vec<GPSPoint>,
}

impl Edge {
    //fn distance() -> f64;
}

struct Vertex {
    // Will be used to display key locations to UI
    name: String,

    // Will be used to identify adjacent nodes and edges
    gps_point: GPSPoint,

    // Key pointer to determine the shortest path using Dijkstra's Algorithm
    parent_vertex: &'static Vertex,

    // Vertex that is connected to an edge will have the same GPS coordinate of
    // the first or last GPS coordinate of 
    adjacent_vertices: Vec<& 'static Vertex>,
    adjacent_edges: Vec<&'static Edge>,
}

/// Returns (number_of_edges, number_of_vertices) in the KML file
fn number_of_edges_and_vertices_from_kml(file_pointer: &File) -> 
(u32, u32) {

    let mut number_of_edges = 0;
    let mut number_of_vertices = 0; 
    
    let reader = BufReader::new(file_pointer);

    let mut graph_element_found = false;
    
    for line in reader.lines() {
        let line = line.unwrap();

        if line.contains("<Placemark>") {
            graph_element_found = true;
        }
        else if line.contains("</Placemark>") {
            graph_element_found = false;
        }

        if graph_element_found {
            // Then determine whether its edge or vertex
            if line.contains("<name>Line") {
                number_of_edges += 1;
            }
            else if line.contains("<name>Point") || line.contains("<name>") {
                // Covers case of a regular vertex, or a key point in our graph
                number_of_vertices += 1;
            }
        }
    }

    return (number_of_edges, number_of_vertices);                                                
}

/*
pub fn initialize_from_kml_file(&File file_pointer) -> 
(Vec<Vertex>, Vec<Edge>) {

    let (number_of_edges, number_of_vertex) = 
        number_of_edges_and_vertices_from_kml(file_pointer);
}
*/