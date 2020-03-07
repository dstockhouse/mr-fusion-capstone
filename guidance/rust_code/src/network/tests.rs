use super::*;
use std::net::IpAddr;

#[test]
#[should_panic]
fn establish_connection_unable_to_bind() {

    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // The error returned is arbitrary. Just want to make sure that if there is an error,
        // we notify the UI and Panic
        .returning(|_: SocketAddrV4| Err(io::Error::last_os_error())); 

    let _ = establish_connection(GUIDANCE_IP, CONTROL_PORT);
}

#[test]
fn establish_connection_able_to_bind_non_blocking() {
    
    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // Please ignore the type parameter to the input of returning since it is unused to set the
        // return.
        .returning(|_: SocketAddrV4| {

            let mut mock_tcp_listener = MockTcpListener::new();
            mock_tcp_listener.expect_set_nonblocking()
                .returning(|_| Ok(()));

            mock_tcp_listener.expect_accept()
                .returning(|| Ok((
                    MockTcpStream::new(), 
                    SocketAddr::new(IpAddr::V4(GUIDANCE_IP), CONTROL_PORT))
                )); // Return stating non-blocking was successful

            Ok(mock_tcp_listener)
        });

    assert!(establish_connection(GUIDANCE_IP, CONTROL_PORT).is_ok());
}