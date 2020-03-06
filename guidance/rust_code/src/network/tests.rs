use super::*;
use std::io::Result;
use std::net::ToSocketAddrs;

#[test]
fn establish_connection() {
    let mock_tcp_listener = MockTcpListener::new();
    mock_tcp_listener.expect_bind();
}