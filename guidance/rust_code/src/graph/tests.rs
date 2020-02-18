
use crate::graph;
use crate::graph::conversions::IntoTangential;
use crate::graph::TangentialPoint;

#[test]
fn initialize_from_gpx_file_test_triangle() {

    let graph = graph::initialize_from_gpx_file("src/graph/Test Triangle.gpx");

    for row in 0..graph.connection_matrix.len() {
        for column in 0..graph.connection_matrix[0].len() {
            if row == column {
                assert_eq!(graph.connection_matrix[row][column], None);
            }
            else {
                assert_ne!(graph.connection_matrix[row][column], None);
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
    assert_eq!(edge.points[0].long, -112.4484608);
    assert_eq!(edge.points[0].lat, 34.615871);
    assert_eq!(edge.points[1].long, -112.4484635);
    assert_eq!(edge.points[1].lat, 34.6157165);
    assert_eq!(edge.points[2].long, -112.4484742);
    assert_eq!(edge.points[2].lat, 34.6155377);
}

#[test]
fn graph_to_geo_json_string() {
    let graph = graph::initialize_from_gpx_file("src/graph/Test Single Edge.gpx");
    
    let json_string = graph::graph_to_geo_json_string(&graph);
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

    let king_front_entrance = graph::GPSPoint {
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
    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx").into_tangential();

    let origin = graph.vertices.iter()
        .filter(|vertex| 
                    vertex.name.contains("King Engineering (Front Entrance)")
        ).next().unwrap();

    
    assert_eq!(origin.point, 
        TangentialPoint{x: 0.0, y: 0.0, z: 0.0}
    );

}