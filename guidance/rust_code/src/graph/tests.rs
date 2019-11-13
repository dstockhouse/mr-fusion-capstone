

use crate::graph;
use crate::graph::{Vertex, Edge};
use std::fs::File;
use std::io::{prelude::*, BufReader, SeekFrom};
use std::mem::size_of;

#[cfg(test)]
pub(self) fn set_up_empty_graph_with_file_name(file_name_with_path: &str) -> 
(BufReader<File>, Vec<Edge>, Vec<Vertex>) {
    let mut file = File::open(file_name_with_path).unwrap();
    let mut reader = BufReader::new(file);

    let (number_of_edges, number_of_vertices) = 
        graph::number_of_edges_and_vertices_from_buffer(&mut reader);

    let (edges,
         vertices) = (
                        Vec::with_capacity(
                            size_of::<Edge>() * number_of_edges as usize
                        ),
                        Vec::with_capacity(
                            size_of::<Vertex>() * number_of_vertices as usize
                        )
                    );
    
    reader.seek(SeekFrom::Start(0)).unwrap();
    
    (reader, edges, vertices)
}




#[test]
fn number_of_edges_and_vertices_from_buffer() {
    let (mut reader, _, _) = set_up_empty_graph_with_file_name("src/graph/Test Triangle.kml");

    let (number_of_edges, number_of_vertices) = 
        graph::number_of_edges_and_vertices_from_buffer(&mut reader);

    assert_eq!(number_of_edges, 3);
    assert_eq!(number_of_vertices, 3);
}

#[test]
fn add_gps_points_to_graph() {
    let (mut reader, mut empty_edges, mut empty_vertices) = 
        set_up_empty_graph_with_file_name("src/graph/Test Single Edge.kml");

    graph::add_gps_points_to_graph(&mut reader, &mut empty_edges, 
                                   &mut empty_vertices);
}

#[test]
fn parse_gps_string() {
    let gps_string = 
        String::from("          -112.4484635,34.6157165,0");
    
    let (lat, long) = graph::parse_gps_string(gps_string);

    assert_eq!(lat, -112.4484635);
    assert_eq!(long, 34.6157165);
}

#[test]
fn number_of_gps_points_for_edge() {
    // setup
    let (mut reader, _edges, _vertices) = 
        set_up_empty_graph_with_file_name("src/graph/Test Single Edge.kml");
    
    let mut lines = reader.by_ref().lines();
    let mut line = lines.next().unwrap().unwrap();

    // Explicate setup of the function precondition
    while !line.contains("<name>Line"){
        line = lines.next().unwrap().unwrap();
    }
    while !line.contains("<coordinates>") {
        line = lines.next().unwrap().unwrap();
    }

    let gps_points_for_edge = graph::number_of_gps_points_for_edge(&mut reader);
    
    assert_eq!(gps_points_for_edge, 3);
    assert_eq!(reader.lines().next().unwrap().unwrap(),
        "          -112.4484608,34.615871,0")
    
}

#[test]
fn initialize_from_kml_file() {

    let (edges, vertices) = 
        graph::initialize_from_kml_file("src/graph/Test Triangle.kml");

    assert_eq!(edges.len(), 3);
    assert_eq!(vertices.len(), 3);

    for vertex in vertices.iter() {
        assert!(vertex.adjacent_vertices.len() > 1, "There exist an edge of
                                                     degree less than 2");
    }
}

