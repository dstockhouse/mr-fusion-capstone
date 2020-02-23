use crate::graph;
use crate::Error;
use crate::graph::{MatrixIndex, EdgeIndex, VertexIndex};
use crate::graph::conversions::IntoTangential;

#[test]
fn robot_on_graph_false_case() {

    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx");

    let middle_of_watson_lake = graph::GPSPointDeg {
        lat: 34.351398,
        long: -112.250598,
        height: 1565.0
    }.into_tangential();

    let robot_on_path = graph.closest_edge_to(&middle_of_watson_lake);

    assert_eq!(robot_on_path, Err(Error::PathPlanningNotOnMap));
}

#[test]
fn robot_on_graph_true_case() {

    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx");
    let king_front_entrance_edge_index = graph.edges.iter()
        .enumerate()
        .filter(|(_, edge)| edge.name.contains("Line 26"))
        .map(|(edge_index, _)| EdgeIndex(edge_index))
        .next().unwrap();
    let expected_matrix_index = graph.connection_matrix_index_from(king_front_entrance_edge_index);

    let king_front_entrance = graph::GPSPointDeg {
        lat: 34.6147979,
        long: -112.4509615,
        height: 1582.3
    }.into_tangential();

    let matrix_index = graph.closest_edge_to(&king_front_entrance);

    assert_eq!(expected_matrix_index, matrix_index);
    
}

#[test]
fn connection_matrix_index_from_edge_index_single_edge() {
    
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    let edge_index = EdgeIndex(0);

    let matrix_index = graph.connection_matrix_index_from(edge_index);

    assert_eq!(matrix_index, Ok(MatrixIndex {
        ith: VertexIndex(0), 
        jth: VertexIndex(1)
    }));
}

#[test]
fn connection_matrix_index_from_edge_not_in_connection_matrix() {
    
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    let faulty_edge_index = EdgeIndex(1);

    let matrix_index = graph.connection_matrix_index_from(faulty_edge_index);

    assert_eq!(matrix_index, Err(Error::PathPlanningEdgeIndexNotInConnectionMatrix));
}

#[test]
fn shortest_path() {

    // Preview this map to see how the names correspond to each vertex and edge
    let mut graph = graph::initialize_from_gpx_file("src/graph/Test Triangle.gpx");

    // Indices of the vertices get set in the order in which they are read from the gps file.
    let vertex_with_name_point_2 = VertexIndex(1);
    let vertex_with_name_point_3 = VertexIndex(2);

    let shortest_path = graph.shortest_path(
        vertex_with_name_point_2, 
        vertex_with_name_point_3
    ).unwrap();
    let mut shortest_path = shortest_path.iter();

    let expected_matrix_index = &MatrixIndex {
        ith: VertexIndex(2),
        jth: VertexIndex(1)
    };
    let expected_edge_at_matrix_index = Some(EdgeIndex(1));

    let actual_matrix_index = shortest_path.next().unwrap();
    let actual_edge_at_matrix_index = graph.connection_matrix[(actual_matrix_index.ith.0, actual_matrix_index.jth.0)];
    // Assertions
    assert_eq!(expected_matrix_index, actual_matrix_index);
    assert_eq!(expected_edge_at_matrix_index, actual_edge_at_matrix_index);

    let expected_matrix_index = None;
    let actual_matrix_index = shortest_path.next();

    assert_eq!(expected_matrix_index, actual_matrix_index);
}

