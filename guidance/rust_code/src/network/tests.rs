use super::*;
use crate::ui::*;
use std::io;
use std::net::IpAddr;


// Some of the mocks are subject to race conditions :( to remedy this, let only run our tests in 1 thread. 
// See following command.
//
// cargo test -- --ignored --test-threads=1
//
// Note: this has a negligible affect on runtime when compared to the time to compile tests.
#[test]
#[should_panic]
fn establish_connection_unable_to_bind() {

    TO_UI.lock().unwrap().expect_write()
        .times(1) // An assertion that the write method to UI was called 1 time.
        .returning(|_| Ok(0));

    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // The error returned is arbitrary. Just want to make sure that if there is an error,
        // we notify the UI and Panic
        .returning(|_: SocketAddrV4| Err(io::Error::last_os_error())); 

    let socket = SocketAddrV4::new(GUIDANCE_IP, CONTROL_PORT);
    let _ = establish_connection(socket);
}

#[test]
fn establish_connection_able_to_bind_and_accept() {
    
    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // Please ignore the type parameter to the input of returning since it is unused to set the
        // return.
        .returning(|_: SocketAddrV4| {

            let mut mock_tcp_listener = MockTcpListener::new();

            mock_tcp_listener.expect_accept()
                .returning(|| Ok((
                    MockTcpStream::new(), 
                    SocketAddr::new(IpAddr::V4(GUIDANCE_IP), CONTROL_PORT))
                ));

            Ok(mock_tcp_listener)
        });
    
    let socket = SocketAddrV4::new(GUIDANCE_IP, CONTROL_PORT);
    
    establish_connection(socket);
    // Assertion is this path does not panic
    
}

#[test]
#[should_panic]
fn establish_connection_unable_to_accept() {
    
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

            mock_tcp_listener.expect_accept()
                .returning(|| Err(io::Error::last_os_error())); // Return stating non-blocking was successful

            Ok(mock_tcp_listener)
        });

        let socket = SocketAddrV4::new(GUIDANCE_IP, CONTROL_PORT);
        establish_connection(socket);
}

// This mocking library has been giving me grief. Ignore this test because some globals are not
// getting unset between tests, even when explicitly running them in one thread.
#[test]
#[ignore]
fn network_setup() {
    
    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // Please ignore the type parameter to the input of returning since it is unused to set the
        // return.
        .returning(|_: SocketAddrV4| {

            let mut mock_tcp_listener = MockTcpListener::new();

            mock_tcp_listener.expect_accept()
                .returning(|| Ok((
                    MockTcpStream::new(), 
                    SocketAddr::new(IpAddr::V4(GUIDANCE_IP), CONTROL_PORT))
                ));

            Ok(mock_tcp_listener)
        });

    TO_UI.lock().unwrap().expect_write()
        .times(0) // Asserting the UI will never be called.
        .returning(|_| Ok(0));
        
    let _tcp_streams = TcpStreams::setup();
    // Assertion does not panic
}