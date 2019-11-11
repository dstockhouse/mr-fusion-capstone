mod graph;
use std::fs::File;
use std::io::prelude::*;

fn main() {
    let mut file = File::open("Shool Map.kml").unwrap();
    
    graph::initialize_from_kml_file(&mut file);
}
