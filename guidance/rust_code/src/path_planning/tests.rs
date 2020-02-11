use crate::graph;
use crate::path_planning;
use crate::Error;

#[test]
fn robot_on_graph_false_case() {
    let graph = graph::initialize_from_kml_file("src/graph/School Map.kml");
    let middle_of_watson_lake = graph::GPSPoint {
        latitude: 34.351398,
        longitude: -112.250598,
        height: 1565.0
    };

    let robot_on_path = path_planning::robot_on_edge(&graph, &middle_of_watson_lake);

    assert_eq!(robot_on_path, Err(Error::RobotNotOnMap));
}

#[test]
fn robot_on_graph_true_case() {
    let graph = graph::initialize_from_kml_file("src/graph/School Map.kml");
    let king_front_entrance = graph::GPSPoint {
        latitude: 34.365316,
        longitude: -112.270342,
        height: 1582.341
    };

    let robot_on_path = path_planning::robot_on_edge(&graph, &king_front_entrance);

    assert_eq!(robot_on_path, Ok(()));
}
