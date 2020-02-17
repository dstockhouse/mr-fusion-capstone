mod graph;
mod error;
mod states;
mod path_planning;
mod constants;

use std::fs;

use error::Error;
use states::States;



fn main() {
    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx");

    // Writing the graph back to disk for visual confirmation
    let geo_json_string = graph::graph_to_geo_json_string(&graph);
    fs::write("school_map.geojson", geo_json_string).unwrap();

    // TODO: Communication here

    let mut current_state: Result<States, Error> = Ok(States::Wait);


    while current_state != Ok(States::Shutdown) {
        match current_state {
            Err(err) => current_state = err.handle(),
            Ok(_) => ()
        }

    }
}
