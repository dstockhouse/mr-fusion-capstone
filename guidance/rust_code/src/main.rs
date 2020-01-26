use std::fs;
mod graph;


fn main() {
    let graph = graph::initialize_from_kml_file("src/graph/School Map.kml");

    // Writing the graph back to disk for visual confirmation
    let geo_json_string = graph::graph_to_geo_json_string(&graph);
    fs::write("school_map.geojson", geo_json_string).unwrap();
}
