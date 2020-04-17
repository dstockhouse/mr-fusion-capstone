mod graph;
mod error;
mod states;
mod path_planning;
mod constants;
mod network;
mod ui;
mod traversal;

use std::fs;
use std::io::Write;
use std::net::{SocketAddrV4, Ipv4Addr, TcpListener};
use std::io::Read;
use std::thread;
use std::time::Duration;

use error::Error;
use states::States;
use graph::conversions;
use ui::TO_UI;

fn main() {

    let graph = graph::initialize_from_gpx_file("src/graph/Test Partial School Map.gpx");

    {
        TO_UI.lock().unwrap().write_all(b"hello").unwrap();
    } // Curly braces are here to drop the lock to the UI
    
    // This function will block waiting for nav, control, and image processing.
    // Blocking will occur in the order listed.
    let tcp_streams = network::TcpStreams::setup();

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
