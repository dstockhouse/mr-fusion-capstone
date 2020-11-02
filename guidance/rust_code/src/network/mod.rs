use std::io::Write;
use std::net::{Ipv4Addr, SocketAddrV4};

use crate::ui::TO_UI;
use cfg_if::cfg_if;

cfg_if! {

    // This can be thought of as a #ifdef in c. In this case, if cargo test is run, then compile the
    // following code contained within the if statement. This mocks networking code reliant on
    // hardware.
    if #[cfg(test)] {

        use mockall::*;
        use std::io::Result;
        use std::net::{ToSocketAddrs, SocketAddr};

        mock! {
            pub TcpStream {}
        }

        use MockTcpStream as TcpStream;

        mock! {
            pub TcpListener {
                fn bind<A: ToSocketAddrs + 'static>(addr: A) -> Result<TcpListener>;
                fn set_nonblocking(&self, nonblocking: bool) -> Result<()>;
                fn accept(&self) -> Result<(TcpStream, SocketAddr)>;
            }
        }

        use MockTcpListener as TcpListener;
    }

    else {
        // cargo run or cargo build was supplied to the terminal. Compile the real code.
        use std::net::{TcpListener, TcpStream};
    }
}

const GUIDANCE_IP: Ipv4Addr = Ipv4Addr::new(192, 168, 1, 1);
const CONTROL_PORT: u16 = 31401;
const NAV_PORT: u16 = 31402;
const IMAG_PROC_PORT: u16 = 31403;

pub struct TcpStreams {
    nav: TcpStream,
    control: TcpStream,
    imag_proc: TcpStream,
}

impl TcpStreams {
    pub fn setup() -> Self {
        TcpStreams {
            nav: establish_connection(SocketAddrV4::new(GUIDANCE_IP, NAV_PORT)),
            control: establish_connection(SocketAddrV4::new(GUIDANCE_IP, CONTROL_PORT)),
            imag_proc: establish_connection(SocketAddrV4::new(GUIDANCE_IP, IMAG_PROC_PORT)),
        }
    }
}

pub(self) fn establish_connection(socket: SocketAddrV4) -> TcpStream {
    let tcp_listener = match TcpListener::bind(socket) {
        Ok(tcp_listener) => tcp_listener,
        Err(error) => {
            let message = format!("Unable to bind to socket: {}", error);

            // Using unwrap since we are guaranteed to get the mutex. Since we only have one thread.
            let mut to_ui = TO_UI.lock().unwrap();

            to_ui
                .write_all(message.as_bytes())
                .expect("Failed to send bind failure to UI");

            // droping ui lock to avoid poison errors in UI test
            drop(to_ui);
            panic!("{}", message)
        }
    };

    match tcp_listener.accept() {
        Ok((tcp_stream, _socket_addr)) => tcp_stream,
        Err(error) => {
            let message = format!("Guidance unable to accept: {}", error);
            let mut to_ui = TO_UI.lock().unwrap();
            to_ui // Acquired mutex
                .write_all(message.as_bytes()) // Sending message to UI
                .expect("Failed to send tcp accept error message to UI");

            // Dropping the lock to the UI so other tests wont have poison errors
            drop(to_ui);
            panic!(message);
        }
    }
}

#[cfg(test)]
mod tests;
