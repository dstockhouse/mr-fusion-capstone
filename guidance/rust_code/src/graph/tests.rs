
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