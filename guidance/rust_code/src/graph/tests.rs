
use crate::graph;
use crate::graph::conversions::{IntoTangential, geo_json_string};
use crate::graph::*;

#[test]
fn initialize_from_gpx_file_test_triangle() {

    let graph = graph::initialize_from_gpx_file("src/graph/Test Triangle.gpx");

    // Asserting the the diagonal of the matrix is none, meaning that no vertex has an edge that
    // connects to itself. Anything not along the diagonal of the matrix should be populated with a 
    // value in this specific case.
    for row in 0..graph.connection_matrix.nrows() {
        for column in 0..graph.connection_matrix.ncols() {
            if row == column {
                assert_eq!(graph.connection_matrix[(row, column)], None);
            }
            else {
                assert_ne!(graph.connection_matrix[(row, column)], None);
            }
        }
    }
}

#[test]
fn full_graph_no_dangling_edge() {
    let graph = graph::initialize_from_gpx_file("src/graph/Full School Map.gpx");

    // Checking that no edge was left unconnected by asserting that the connection matrix is 
    // symmetrical across its diagonal.
    for row_i in 0..graph.connection_matrix.nrows() {
        for col_i in 0..graph.connection_matrix.ncols() {
            if graph.connection_matrix[(row_i, col_i)].is_some() {
                assert!(graph.connection_matrix[(col_i, row_i)].is_some())
            }
        }
    }
}

#[test]
fn initialize_from_gpx_file_single_edge() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");

    let edges = &graph.edges;
    let edge = &edges[0];

    assert_eq!(edges.len(), 1);
    assert_eq!(edge.points.len(), 3);
    assert_eq!(edge.points[0].gps.long, -112.4484608);
    assert_eq!(edge.points[0].gps.lat, 34.615871);
    assert_eq!(edge.points[1].gps.long, -112.4484635);
    assert_eq!(edge.points[1].gps.lat, 34.6157165);
    assert_eq!(edge.points[2].gps.long, -112.4484742);
    assert_eq!(edge.points[2].gps.lat, 34.6155377);
}

#[test]
fn graph_to_geo_json_string() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    
    let json_string = geo_json_string(&graph, &graph);
    let expected_json_string = r#"{"features":[{"geometry":{"coordinates":[[-112.4484608,34.615871],[-112.4484635,34.6157165],[-112.4484742,34.6155377]],"type":"LineString"},"id":"Line 3","properties":{},"type":"Feature"},{"geometry":{"coordinates":[-112.4484608,34.615871],"type":"Point"},"id":"Point 1","properties":{},"type":"Feature"},{"geometry":{"coordinates":[-112.4484742,34.6155377],"type":"Point"},"id":"Point 2","properties":{},"type":"Feature"}],"type":"FeatureCollection"}"#;

    assert_eq!(json_string, expected_json_string);

}

#[test]
fn distance() {
    
    let origin = graph::TangentialPoint {
        vector: Vector3::new(0.0, 0.0, 0.0)
    };

    let point = graph::TangentialPoint {
        vector: Vector3::new(1.0, 1.0, 1.0)
    };

    let distance = origin.distance(&point);

    assert_eq!(distance, 3.0_f64.sqrt());
    
}

#[test]
fn into_tangential() {

    let king_front_entrance = graph::GPSPointDeg {
        lat: 34.6147979,
        long: -112.4509615,
        height: 1582.3
    };

    assert_eq!(king_front_entrance.into_tangential(),
        graph::TangentialPoint {
            vector: Vector3::new(0.0, 0.0, 0.0)
        }
    )

}

#[test]
fn tangential_sub() {
    let origin = &graph::TangentialPoint {
        vector: Vector3::new(0.0, 0.0, 0.0)
    };
    let point = &graph::TangentialPoint {
        vector: Vector3::new(1.0, 2.0, 3.0)
    };

    assert_eq!(point - origin,
        TangentialPoint {
            vector: Vector3::new(1.0, 2.0, 3.0)
        }
    )
}   

#[test]
fn graph_into_tangential() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Partial School Map.gpx");

    let origin = graph.vertices.iter()
        .filter(|vertex| 
                    vertex.name.contains("King Engineering (Front Entrance)")
        ).next().unwrap();

    
    assert_eq!(origin.point.tangential, 
        TangentialPoint{
            vector: Vector3::new(0.0, 0.0, 0.0)
        }
    );

}

#[test]
fn into_tangential_correct_distances() {

    let king_start_edge = graph::GPSPointDeg {
        lat: 34.6147979,
        long: -112.4509615,
        height: 1582.341
    }.into_tangential();

    let king_end_edge = graph::GPSPointDeg {
        lat: 34.6148752,
        long: -112.4509389,
        height: 1581.907
    }.into_tangential();

    // Expected distance is about 8.8382m
    let distance_between_points = king_end_edge.distance(&king_start_edge);

    assert!(
        distance_between_points > 8.0
            &&
        distance_between_points < 9.0
    );
    
}

#[test]
fn edge_initialization() {
    let points = vec![
        Point {
            tangential: TangentialPoint {
                vector: Vector3::new(0.0, 0.0, 0.0)
            },
            gps: GPSPointDeg{lat: -1.0, long: -1.0, height: -1.0} // Unimportant for this test
        } ,
        Point {
            tangential: TangentialPoint {
                vector: Vector3::new(1.0, 1.0, 1.0)
            },
            gps: GPSPointDeg{lat: -1.0, long: -1.0, height: -1.0} // Unimportant for this test
        }
    ];
    
    let name = String::from("the slight");

    assert_eq!(Edge::new(name.clone(), points.clone()),
        Edge {
            name,
            points,
            distance: 3.0_f64.sqrt()
        }
    )
}

#[test]
fn edge_from_connection_index() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    let vertex_index_0 = 0;
    let vertex_index_1 = 1;

    let matrix_index = MatrixIndex {
        ith: vertex_index_0,
        jth: vertex_index_1,
    };

    assert_eq!(&graph.edges[0], matrix_index.edge(&graph));
    
}

#[test]
fn vertices_from_connection_matrix() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    let vertex_index_0 = 0;
    let vertex_index_1 = 1;

    let matrix_index = MatrixIndex {
        ith: 0,
        jth: 1,
    };

    assert_eq!(
        matrix_index.vertices(&graph),
        (&graph.vertices[0], &graph.vertices[1])
    )
}   