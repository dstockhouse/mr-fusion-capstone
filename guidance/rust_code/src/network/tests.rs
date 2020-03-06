use super::*;
use std::io::Result;
use std::net::ToSocketAddrs;

use mockall::*;

mock! {
    pub TcpListener {
        pub fn bind<A: ToSocketAddrs + 'static>(addr: A) -> Result<TcpListener>;
    }
}

#[test]
fn estblish_connection_unable_to_bind() {
    let mock_tcp_listener = MockTcpListener::new();

    mock_tcp_listener.expect_bind()
}