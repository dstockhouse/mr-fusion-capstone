mod graph;
mod error;
mod states;
mod path_planning;
mod constants;

use std::fs;
use std::net::{SocketAddrV4, Ipv4Addr, TcpListener};
use std::io::Read;

use error::Error;
use states::States;
use graph::conversions;

fn main() {

    let loopback = Ipv4Addr::new(169, 254, 243, 252);
    let socket = SocketAddrV4::new(loopback, 0);
    let listener = TcpListener::bind(socket).unwrap();
    let port = listener.local_addr().unwrap();
    println!("Listening on {}, access this port to end the program", port);
    let (mut tcp_stream, addr) = listener.accept().unwrap(); //block  until requested
    println!("Connection received! {:?} is sending data.", addr);
    let mut input = String::new();
    let _ = tcp_stream.read_to_string(&mut input).unwrap();
    println!("{:?} says {}", addr, input);
    
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
