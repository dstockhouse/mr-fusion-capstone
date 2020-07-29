use crate::graph;
use crate::Error;
use crate::graph::{MatrixIndex, Vertex, Vertices, Edges};
use crate::graph::conversions::{IntoTangential};
use crate::path_planning::*;

#[test]
fn robot_on_graph_false_case() {

    let graph = graph::initialize_from_gpx_file("src/graph/Test Partial School Map.gpx");

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

    let graph = graph::initialize_from_gpx_file("src/graph/Test Partial School Map.gpx");
    let king_front_entrance_edge_index = graph.edges.iter()
        .enumerate()
        .filter(|(_, edge)| edge.name.contains("Line 26"))
        .map(|(edge_index, _)| edge_index)
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
fn robot_not_on_graph_within_radius() {

    let graph = graph::initialize_from_gpx_file("src/graph/Test Partial School Map.gpx");

    // Enter lat long in google maps if you wish to verify location of point with our school map
    let close_point = GPSPointDeg {
        lat: 34.6148261, 
        long: -112.4508876,
        height: 1582.0
    }.into_tangential();

    let expected_edge = graph.closest_edge_to(&close_point)
        .unwrap()
        .edge(&graph); // Taking the matrix index and returning a reference to the edge

    assert!(expected_edge.name.contains("Line 28"));

}

#[test]
fn connection_matrix_index_from_edge_index_single_edge() {
    
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    let edge_index = 0;
    let vertex_index_0 = 0;
    let vertex_index_1 = 1;

    let matrix_index = graph.connection_matrix_index_from(edge_index);

    assert_eq!(matrix_index, Ok(MatrixIndex {
        ith: vertex_index_0, 
        jth: vertex_index_1
    }));
}

#[test]
fn connection_matrix_index_from_edge_not_in_connection_matrix() {
    
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    let faulty_edge_index = 1;

    let matrix_index = graph.connection_matrix_index_from(faulty_edge_index);

    assert_eq!(matrix_index, Err(Error::PathPlanningEdgeIndexNotInConnectionMatrix));
}

#[test]
fn shortest_path_triangle() {

    // Preview this map to see how the names correspond to each vertex and edge
    let mut graph = graph::initialize_from_gpx_file("src/graph/Test Triangle.gpx");

    // Indices of the vertices get set in the order in which they are read from the gpx file.
    let start = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("Point 2"))
        .unwrap();
    let end = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("Point 3"))
        .unwrap();

    let shortest_path = graph.shortest_path(
        start, 
        end
    ).unwrap();

    let edges_chosen = shortest_path.edges(&graph);
    let mut edges_chosen = edges_chosen.iter();

    let vertices_chosen = shortest_path.vertices(&graph);
    let mut vertices_chosen = vertices_chosen.iter();

    assert!(edges_chosen.next().unwrap().name.contains("Line 5"));
    assert_eq!(edges_chosen.next(), None);

    assert!(vertices_chosen.next().unwrap().name.contains("Point 2"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 3"));
    assert_eq!(vertices_chosen.next(), None);
    
}

#[test]
fn shortest_path_school_map() {

    let mut graph = graph::initialize_from_gpx_file("src/graph/Test Partial School Map.gpx");

    let king_front_entrance = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("King Engineering (Front Entrance)"))
        .unwrap();

    let library = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("Library"))
        .unwrap();

    let path = graph.shortest_path(
        king_front_entrance, 
        library
    ).unwrap();

    let edges_chosen = path.edges(&graph);
    let mut edges_chosen = edges_chosen.iter();

    let vertices_chosen = path.vertices(&graph);
    let mut vertices_chosen = vertices_chosen.iter();

    // See map preview or write path to GeoJson to visually confirm assertions
    assert!(edges_chosen.next().unwrap().name.contains("Line 26"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 32"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 30"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 35"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 36"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 37"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 38"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 39"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 40"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 41"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 42"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 16"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 17"));
    assert_eq!(edges_chosen.next(), None);

    assert!(vertices_chosen.next().unwrap().name.contains("King Engineering (Front Entrance)"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 25"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 17"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 18"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 28"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 29"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 30"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 31"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 32"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 33"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 34"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 12"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 5"));
    assert!(vertices_chosen.next().unwrap().name.contains("Library"));
    assert_eq!(vertices_chosen.next(), None);

}

#[test]
fn shortest_path_no_path() {

    // Use map preview for visual verification of points chosen for test
    let mut graph = graph::initialize_from_gpx_file("src/graph/Test Dijkstra.gpx");
    let start_vertex_index = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("Point 12"))
        .unwrap();
    let end_vertex_index = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("Point 16"))
        .unwrap();

    let shortest_path = graph.shortest_path(
        start_vertex_index,
        end_vertex_index
    );

    assert_eq!(shortest_path, Err(Error::PathPlanningPathDoesNotExist));
}

#[test]
fn shortest_path_dijkstra_graph() {
    // Use map preview for visual verification of points chosen for test
    let mut graph = graph::initialize_from_gpx_file("src/graph/Test Dijkstra.gpx");
    let start_vertex_index = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("Point 12"))
        .unwrap();
    let end_vertex_index = graph.vertices.iter()
        .position(|vertex| vertex.name.contains("Point 5"))
        .unwrap();

    let shortest_path = graph.shortest_path(
        start_vertex_index,
        end_vertex_index
    ).unwrap();

    let vertices_chosen = shortest_path.vertices(&graph);
    let mut vertices_chosen = vertices_chosen.iter();

    let edges_chosen = shortest_path.edges(&graph);
    let mut edges_chosen = edges_chosen.iter();

    assert!(vertices_chosen.next().unwrap().name.contains("Point 12"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 1"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 17"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 16"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 4"));
    assert!(vertices_chosen.next().unwrap().name.contains("Point 5"));
    assert_eq!(vertices_chosen.next(), None);

    assert!(edges_chosen.next().unwrap().name.contains("Line 13"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 18"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 19"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 20"));
    assert!(edges_chosen.next().unwrap().name.contains("Line 14"));

}

#[test]
fn vertices_for_path_single_edge() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    let vertex_index_0 = 0;
    let vertex_index_1 = 1;

    let path = Path {
        indices: vec![
            MatrixIndex {
                ith: vertex_index_0,
                jth: vertex_index_1,
            }
        ]
    };

    assert_eq!(
        path.vertices(&graph),
        vec![&graph.vertices[0], &graph.vertices[1]]
    )
}

#[test]
fn vertices_for_path_triangle() {

    let graph = graph::initialize_from_gpx_file("src/graph/Test Triangle.gpx");
    let vertex_index_0 = 0;
    let vertex_index_1 = 1;
    let vertex_index_2 = 2;

    let path = Path {
        indices: vec![
            MatrixIndex {
                ith: vertex_index_0,
                jth: vertex_index_1,
            },
            MatrixIndex {
                ith: vertex_index_1,
                jth: vertex_index_2,
            }
        ]
    };

    let expected_vertices: Vec<&Vertex> = graph.vertices.iter().collect();

    assert_eq!(path.vertices(&graph), expected_vertices)
}

