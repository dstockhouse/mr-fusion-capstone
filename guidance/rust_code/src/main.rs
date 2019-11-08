mod graph;
use std::fs::File;
use std::io::prelude::*;

fn main() {
    let file = File::open("Shool Map.kml").unwrap();
    graph::number_of_edges_and_vertices_from_kml(&file);
}
