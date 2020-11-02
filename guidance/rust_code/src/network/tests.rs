use super::*;
use crate::ui::*;
use std::io;
use std::net::IpAddr;

// Some tests are desgined to panic. Unfortuneately, those tests share a state with the UI and can 
// cause other tests designed not to panic, to panic. To remedy this, we run the non-panicing tests in parrallel,
// and then run the panicing tests in a single thread using the following command.
//
// cargo test -- --ignored --test-threads=1
//
// Note: this has a negligible affect on runtime when compared to the time to compile tests.
#[test]
#[ignore = "single threaded only"]
#[should_panic]
fn establish_connection_unable_to_bind() {
    let mut to_ui = TO_UI.lock().unwrap();
    to_ui
        .expect_write()
        .times(1) // An assertion that the write method to UI was called 1 time.
        .returning(|_| Ok(0));

    let mock_bind = MockTcpListener::bind_context();
    mock_bind
        .expect()
        // The error returned is arbitrary. Just want to make sure that if there is an error,
        // we notify the UI and Panic
        .returning(|_: SocketAddrV4| Err(io::Error::last_os_error()));

    let socket = SocketAddrV4::new(GUIDANCE_IP, CONTROL_PORT);
    let _ = establish_connection(socket);
}

#[test]
fn establish_connection_able_to_bind_and_accept() {
    // Aquiring lock as sycronization for mock bind
    let _ = TO_UI.lock().unwrap();

    // Making acquring the lock succeed regardless if another thread panics
    // with the lock. a.k.a poisoning.
    let mock_bind = MockTcpListener::bind_context();

    mock_bind
        .expect()
        // Please ignore the type parameter to the input of returning since it is unused to set the
        // return.
        .returning(|_: SocketAddrV4| {
            let mut mock_tcp_listener = MockTcpListener::new();

            mock_tcp_listener.expect_accept().returning(|| {
                Ok((
                    MockTcpStream::new(),
                    SocketAddr::new(IpAddr::V4(GUIDANCE_IP), CONTROL_PORT),
                ))
            });

            Ok(mock_tcp_listener)
        });

    let socket = SocketAddrV4::new(GUIDANCE_IP, CONTROL_PORT);

    establish_connection(socket);
    // Assertion is this path does not panic
}

#[test]
#[ignore = "single threaded only"]
#[should_panic]
fn establish_connection_unable_to_accept() {
    let mut to_ui = TO_UI.lock().unwrap();
    to_ui
        .expect_write()
        .times(1) // An assertion that the write method to UI was called 1 time.
        .returning(|_| Ok(0));

    drop(to_ui);

    let mock_bind = MockTcpListener::bind_context();
    mock_bind
        .expect()
        // Please ignore the type parameter to the input of returning since it is unused to set the
        // return.
        .returning(|_: SocketAddrV4| {
            let mut mock_tcp_listener = MockTcpListener::new();

            mock_tcp_listener
                .expect_accept()
                .returning(|| Err(io::Error::last_os_error())); // Return stating non-blocking was successful

            Ok(mock_tcp_listener)
        });

    let socket = SocketAddrV4::new(GUIDANCE_IP, CONTROL_PORT);
    establish_connection(socket);
}


#[test]
fn network_setup() {
    let mut to_ui = TO_UI.lock().unwrap();
    to_ui
        .expect_write()
        .times(0) // Asserting the UI will never be called.
        .returning(|_| Ok(0));

    let mock_bind = MockTcpListener::bind_context();
    mock_bind
        .expect()
        // Please ignore the type parameter to the input of returning since it is unused to set the
        // return.
        .returning(|_: SocketAddrV4| {
            let mut mock_tcp_listener = MockTcpListener::new();

            mock_tcp_listener.expect_accept().returning(|| {
                Ok((
                    MockTcpStream::new(),
                    SocketAddr::new(IpAddr::V4(GUIDANCE_IP), CONTROL_PORT),
                ))
            });

            Ok(mock_tcp_listener)
        });

    let _tcp_streams = TcpStreams::setup();
    // Assertion does not panic
}
