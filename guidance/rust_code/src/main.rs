// remove this once if the state machine ever gets implemented
#![allow(dead_code)] 
#![cfg_attr(feature = "nightly", feature(test))]

mod constants;
mod error;
mod graph;
mod network;
mod path_planning;
mod states;
mod traversal;
mod ui;

use std::fs;
use std::io::Read;
use std::io::Write;
use std::net::{Ipv4Addr, SocketAddrV4, TcpListener};
use std::thread;
use std::time::Duration;

use error::Error;
use graph::conversions;
use states::States;
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
                States::Shutdown => break,
            },
        }
    }
}
