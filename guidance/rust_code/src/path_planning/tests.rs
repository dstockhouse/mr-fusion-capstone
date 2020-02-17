use crate::graph;
use crate::path_planning;
use crate::Error;
use crate::graph::conversions::IntoTangential;

#[test]
fn robot_on_graph_false_case() {

    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx")
        .into_tangential();

    let middle_of_watson_lake = graph::GPSPoint {
        lat: 34.351398,
        long: -112.250598,
        height: 1565.0
    }.into_tangential();

    let robot_on_path = path_planning::robot_on_edge(&graph, &middle_of_watson_lake);

    assert_eq!(robot_on_path, Err(Error::RobotNotOnMap));
}

#[test]
fn robot_on_graph_true_case() {

    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx")
        .into_tangential();

    let king_front_entrance = graph::GPSPoint {
        lat: 34.365316,
        long: -112.270342,
        height: 1582.341
    }.into_tangential();

    let robot_on_path = path_planning::robot_on_edge(&graph, &king_front_entrance);

    assert_eq!(robot_on_path, Ok(()));
    
}
