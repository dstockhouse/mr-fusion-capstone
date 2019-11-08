
#[cfg(test)]
use crate::graph;
use std::fs::File;
use std::io::prelude::*;

#[test]
fn test_number_of_edges_and_vertices_from_kml() {
    let file = File::open("src/graph/Test Triangle.kml").unwrap();

    let (number_of_edges, number_of_vertices) = 
        graph::number_of_edges_and_vertices_from_kml(&file);

    assert_eq!(number_of_edges, 3);
    assert_eq!(number_of_vertices, 3);
}

#[test]
fn test_initialize_from_kml_file() {
    let file = File::open("src/graph/Test Triangle.kml").unwrap();

    let (edges, vertices) = graph::initialize_from_kml_file(&file);

    assert_eq!(edges.len(), 3);
    assert_eq!(vertices.len(), 3);

    for vertex in vertices.iter() {
        assert!(vertex.adjacent_vertices.len() > 1, "There exist an edge of
                                                     degree less than 2");
    }
}