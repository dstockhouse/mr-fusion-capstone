
#[cfg(test)]
use crate::graph;
use crate::graph::{Vertex, Edge};
use std::fs::File;
use std::io::{prelude::*, BufReader};
use std::mem::size_of;

pub(self) fn set_up_empty_graph_with_file_name(file_name_with_path: &str) -> 
(File, Vec<Edge>, Vec<Vertex>) {
    let mut file = File::open(file_name_with_path).unwrap();

    let (number_of_edges, number_of_vertices) = 
        graph::number_of_edges_and_vertices_from_kml(&mut file);

    let ( edges,
         vertices) = (
                        Vec::with_capacity(
                            size_of::<Edge>() * number_of_edges as usize
                        ),
                        Vec::with_capacity(
                            size_of::<Vertex>() * number_of_vertices as usize
                        )
                    );
    
    return (file, edges, vertices)
}

#[test]
fn test_number_of_edges_and_vertices_from_kml() {
    let mut file = File::open("src/graph/Test Triangle.kml").unwrap();

    let (number_of_edges, number_of_vertices) = 
        graph::number_of_edges_and_vertices_from_kml(&mut file);

    assert_eq!(number_of_edges, 3);
    assert_eq!(number_of_vertices, 3);
}

// TODO: Make an easy file where we can test all the gps points
#[test]
fn test_add_gps_points_to_graph() {
    let mut file = File::open("src/graph/Test Triangle.kml").unwrap();

    let (number_of_edges, number_of_vertices) = 
        graph::number_of_edges_and_vertices_from_kml(&mut file);

    let (mut edges,
         mut vertices) = (
                            Vec::with_capacity(
                                size_of::<Edge>() * number_of_edges as usize
                            ),
                            Vec::with_capacity(
                                size_of::<Vertex>() * number_of_vertices as usize
                            )
                        );

    graph::add_gps_points_to_graph(&mut file, &mut edges, &mut vertices);
}

#[test]
fn test_number_of_gps_points_for_edge() {
    let (mut file, mut edges, _vertices) = 
        set_up_empty_graph_with_file_name("src/graph/Test Single Edge.kml");
    
    let reader = BufReader::new(&file);
    let mut lines = reader.lines();

    while let line = lines.next() {
        let mut line = line.unwrap().unwrap();

        if line.contains("<name>Line") {
            // then an edge has been detected

            // Fast forwarding to coordinates of the edge
            for _ in 0..4 {
                line = lines.next().unwrap().unwrap();
            }

            graph::number_of_gps_points_for_edge(&mut file);
            break;
        }
    }
        //////******************************* */
}

#[test]
fn test_initialize_from_kml_file() {
    let mut file = File::open("src/graph/Test Triangle.kml").unwrap();

    let (edges, vertices) = graph::initialize_from_kml_file(&mut file);

    assert_eq!(edges.len(), 3);
    assert_eq!(vertices.len(), 3);

    for vertex in vertices.iter() {
        assert!(vertex.adjacent_vertices.len() > 1, "There exist an edge of
                                                     degree less than 2");
    }
}

