mod graph;
mod error;
mod states;
mod path_planning;
mod constants;
mod wait;
mod traverse;

use std::fs;

use error::Error;
use states::States;
use graph::conversions::IntoTangential;
use path_planning::plan_path;
use wait::wait;
use traverse::traverse;



fn main() {
    let graph = graph::initialize_from_gpx_file("src/graph/School Map.gpx");

    // Writing the graph back to disk for visual confirmation
    let geo_json_string = graph::graph_to_geo_json_string(&graph);
    fs::write("school_map.geojson", geo_json_string).unwrap();

    let graph = graph.into_tangential();

    // TODO: Communication here

    let mut current_state: Result<States, Error> = Ok(States::Wait);

    loop {
        match current_state {
            Err(err) => current_state = err.handle(),
            Ok(state) => match state {
                States::PlanPath => current_state = plan_path(&graph),
                States::Wait => current_state = wait(),
                States::Traverse => current_state = traverse(),
                States::Shutdown => break
            }
        }
    }
}
