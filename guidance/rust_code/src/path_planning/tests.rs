use crate::graph;
use crate::path_planning;
use crate::Error;

#[test]
fn robot_on_graph_false_case() {

    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx");

}

#[test]
fn robot_on_graph_true_case() {
    unimplemented!();
    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx");

}
