
use crate::graph;
use crate::graph::conversions::{IntoTangential, IntoGeoJson};
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
    
    let json_string = graph.to_geo_json_string(&graph);
    let expected_json_string = r#"{"features":[{"geometry":{"coordinates":[[-112.4484608,34.615871],[-112.4484635,34.6157165],[-112.4484742,34.6155377]],"type":"LineString"},"id":"Line 3","properties":{},"type":"Feature"},{"geometry":{"coordinates":[-112.4484608,34.615871],"type":"Point"},"id":"Point 1","properties":{},"type":"Feature"},{"geometry":{"coordinates":[-112.4484742,34.6155377],"type":"Point"},"id":"Point 2","properties":{},"type":"Feature"}],"type":"FeatureCollection"}"#;

    assert_eq!(json_string, expected_json_string);

}

#[test]
fn distance() {
    
    let origin = graph::TangentialPoint {
        x: 0.0,
        y: 0.0,
        z: 0.0
    };

    let point = graph::TangentialPoint {
        x: 1.0,
        y: 1.0,
        z: 1.0
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
        graph::TangentialPoint{x: 0.0, y: 0.0, z: 0.0}
    )

}

#[test]
fn tangential_sub() {
    let origin = &graph::TangentialPoint{x: 0.0, y: 0.0, z: 0.0};
    let point = &graph::TangentialPoint{x: 1.0, y: 2.0, z: 3.0};

    assert_eq!(point - origin,
        (1.0, 2.0, 3.0)
    )
}   

#[test]
fn graph_into_tangential() {
    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx");

    let origin = graph.vertices.iter()
        .filter(|vertex| 
                    vertex.name.contains("King Engineering (Front Entrance)")
        ).next().unwrap();

    
    assert_eq!(origin.point.tangential, 
        TangentialPoint{x: 0.0, y: 0.0, z: 0.0}
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
            tangential: TangentialPoint{x: 0.0, y: 0.0, z: 0.0},
            gps: GPSPointDeg{lat: -1.0, long: -1.0, height: -1.0} // Unimportant for this test
        } ,
        Point {
            tangential: TangentialPoint{x: 1.0, y: 1.0, z: 1.0},
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

    let matrix_index = MatrixIndex {
        ith: VertexIndex(0),
        jth: VertexIndex(1),
    };

    assert_eq!(&graph.edges[0], matrix_index.edge(&graph));
    
}

#[test]
fn vertices_from_connection_matrix() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");

    let matrix_index = MatrixIndex {
        ith: VertexIndex(0),
        jth: VertexIndex(1),
    };

    assert_eq!(
        matrix_index.vertices(&graph),
        (&graph.vertices[0], &graph.vertices[1])
    )
}   