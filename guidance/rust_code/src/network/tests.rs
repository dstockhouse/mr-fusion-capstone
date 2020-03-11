use super::*;
use crate::ui::*;
use std::net::IpAddr;


// Tests that are ignored are subject to race conditions this is due to the mockall library and
// have opened an issue here https://github.com/asomers/mockall/issues/105 
// We will explicitly run tests marked as #[ignore] using only one thread.
// To run these tests in a single thread execute the following command.
//
// cargo test -- --ignored --test-threads=1
//
#[test]
#[should_panic]
#[ignore]
fn establish_connection_unable_to_bind() {

    TO_UI.lock().unwrap().expect_write()
        .times(1) // An assertion that the write method to UI was called 1 time.
        .returning(|_| Ok(0));

    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // The error returned is arbitrary. Just want to make sure that if there is an error,
        // we notify the UI and Panic
        .returning(|_: SocketAddrV4| Err(io::Error::last_os_error())); 

    let _ = establish_connection(GUIDANCE_IP, CONTROL_PORT);
}

#[test]
#[ignore]
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

#[test]
#[ignore]
fn establish_connection_able_to_bind_blocking() {

    TO_UI.lock().unwrap().expect_write()
        .times(1) // An assertion that the write method to UI was called 1 time.
        .returning(|_| Ok(0));

    // Mocking network connections
    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // Please ignore the type parameter to the input of returning since it is unused to set the
        // return.
        .returning(|_: SocketAddrV4| {

            let mut mock_tcp_listener = MockTcpListener::new();
            mock_tcp_listener.expect_set_nonblocking()
                .returning(|_| Err(io::Error::last_os_error())); // Error is arbitrary

            mock_tcp_listener.expect_accept()
                .returning(|| Ok((
                    MockTcpStream::new(), 
                    SocketAddr::new(IpAddr::V4(GUIDANCE_IP), CONTROL_PORT))
                )); // Return stating non-blocking was successful

            Ok(mock_tcp_listener)
        });

        assert!(establish_connection(GUIDANCE_IP, CONTROL_PORT).is_ok());

}