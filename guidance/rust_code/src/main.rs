mod graph;
mod error;
mod states;
mod path_planning;
mod constants;

use std::fs;

use error::Error;
use states::States;
use graph::conversions;

fn main() {
    
    let graph = graph::initialize_from_gpx_file("src/graph/Test Partial School Map.gpx");

    // Writing the graph back to disk for visual confirmation
    let geo_json_string = conversions::geo_json_string(&graph, &graph);

    fs::write("school_map.geojson", geo_json_string).unwrap();

    // TODO: Communication here
    let mut current_state: Result<States, Error> = Ok(States::Wait);

    loop {
        match current_state {
            Err(err) => unimplemented!(),
            Ok(state) => match state {
                States::PlanPath => unimplemented!(),
                States::Wait => unimplemented!(),
                States::Traverse => unimplemented!(),
                States::Shutdown => break
            }
        }
    }
}
