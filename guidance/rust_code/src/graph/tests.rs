
use crate::graph;
use crate::graph::{Vertex, Edge};
use std::fs::File;
use std::io::{prelude::*, BufReader, SeekFrom};


pub(self) fn set_up_empty_graph_with_file_name(file_name_with_path: &str) -> 
(BufReader<File>, Vec<Edge>, Vec<Vertex>) {
    let mut file = File::open(file_name_with_path).unwrap();
    let mut reader = BufReader::new(file);

    let (number_of_edges, number_of_vertices) = 
        graph::number_of_edges_and_vertices_from_buffer(&mut reader);

    let (edges,
         vertices) = (
                        Vec::with_capacity(number_of_edges as usize),
                        Vec::with_capacity(number_of_vertices as usize)
                    );
    
    reader.seek(SeekFrom::Start(0)).unwrap();
    
    (reader, edges, vertices)
}

pub(self) fn set_up_unconnected_graph_with_file_name(file_name_with_path: &str) ->
(Vec<Edge>, Vec<Vertex>) {

    let file = File::open(file_name_with_path).unwrap();
    // Open the file and store its contents to a buffer in RAM
    let mut reader = BufReader::new(file);

    let (number_of_edges, number_of_vertices) =
        graph::number_of_edges_and_vertices_from_buffer(&mut reader);

    let (mut edges, mut vertices) = (
        Vec::with_capacity(number_of_edges as usize),
        Vec::with_capacity(number_of_vertices as usize),
    );

    graph::add_gps_points_to_edges(&mut reader, &mut edges);
    graph::add_gps_points_to_vertices(&mut reader, &mut vertices);

    (edges, vertices)
}
#[test]
fn add_gps_points_to_edges() {
    let (mut reader, mut edges, _) = 
        set_up_empty_graph_with_file_name("src/graph/Test Single Edge.kml");

    graph::add_gps_points_to_edges(&mut reader, &mut edges);

    let edge = &edges[0];

    assert_eq!(edges.len(), 1);
    assert_eq!(edge.gps_points.len(), 3);
    assert_eq!(edge.gps_points[0].latitude, -112.4484608);
    assert_eq!(edge.gps_points[0].longitude, 34.615871);
    assert_eq!(edge.gps_points[1].latitude, -112.4484635);
    assert_eq!(edge.gps_points[1].longitude, 34.6157165);
    assert_eq!(edge.gps_points[2].latitude, -112.4484742);
    assert_eq!(edge.gps_points[2].longitude, 34.6155377);

}
#[test]
fn add_gps_points_to_vertices() {
    let (mut reader, _, mut vertices) = 
        set_up_empty_graph_with_file_name("src/graph/Test Triangle.kml");

    graph::add_gps_points_to_vertices(&mut reader, &mut vertices);

    assert_eq!(vertices.len(), 3);
    
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
fn parse_gps_string() {
    let gps_string = 
        String::from("          -112.4484635,34.6157165,0");
    
    let (lat, long) = graph::parse_gps_string(&gps_string);

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

    let gps_points_for_edge = graph::number_of_gps_points_for_edge(&mut reader);
    let next_line = reader.lines().next().unwrap().unwrap();
    assert_eq!(gps_points_for_edge, 3);
    assert_eq!(next_line, 
        "      <styleUrl>#line-000000-1200-nodesc</styleUrl>");
    
}

#[test]
fn connect_vertices_with_edges() {
    let (mut edges, mut vertices) = 
        set_up_unconnected_graph_with_file_name("src/graph/Test Single Edge.kml");

    let graph = graph::connect_vertices_with_edges(edges, vertices);

    let row1 = &graph.connection_matrix[0];
    let row2 = &graph.connection_matrix[1];

    assert_eq!(row1, &vec![None, Some(0)]);
    assert_eq!(row2, &vec![Some(0), None]);
}

#[test]
fn initialize_from_kml_file() {

    let graph = graph::initialize_from_kml_file("src/graph/Test Triangle.kml");

    for row in 0..graph.connection_matrix.len() {
        for column in 0..graph.connection_matrix[0].len() {
            if row == column {
                assert_eq!(graph.connection_matrix[row][column], None);
            }
            else {
                assert_ne!(graph.connection_matrix[row][column], None);
            }
        }
    }


}


