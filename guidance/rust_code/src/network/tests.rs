use super::*;
use std::net::IpAddr;
use std::sync::Mutex;

// Since mock bind is a static method, mocking properties are globally defined. This requires
// synchronization in order to guarantee no race conditions while running multiple tests.
// The compiler will allow race conditions should this syncronization be removed. I have
// opened an issue in mockall requesting mocking a static function require the use of the unsafe
// keyword to clue the user of the library of this undefined behavior.
lazy_static! {
    static ref MOCK_BIND_MUTEX: Mutex<()> = Mutex::new(());
}

#[test]
#[should_panic]
fn establish_connection_unable_to_bind() {
    
    // Acquiring the mutex to avoid race conditions when mocking the bind function
    let _m = MOCK_BIND_MUTEX.lock().unwrap();

    let mock_bind = MockTcpListener::bind_context();
    mock_bind.expect()
        // The error returned is arbitrary. Just want to make sure that if there is an error,
        // we notify the UI and Panic
        .returning(|_: SocketAddrV4| Err(io::Error::last_os_error())); 

    let _ = establish_connection(GUIDANCE_IP, CONTROL_PORT);
}

#[test]
fn establish_connection_able_to_bind_non_blocking() {
    
    let _m = MOCK_BIND_MUTEX.lock().unwrap();

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