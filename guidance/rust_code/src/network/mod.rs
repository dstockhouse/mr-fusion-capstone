use std::net::{Ipv4Addr, SocketAddrV4, SocketAddr};
use std::io;
use std::io::Write;

use cfg_if::cfg_if;
use crate::ui::TO_UI;


cfg_if! {

    // This can be thought of as a #ifdef in c. In this case, if cargo test is run, then compile the 
    // following code contained within the if statement. This mocks networking code reliant on 
    // hardware.
    if #[cfg(test)] {

        use mockall::*;
        use std::io::Result;
        use std::net::ToSocketAddrs;

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

// This function is not finished yet
pub(self) fn establish_connection(address: Ipv4Addr, port: u16) -> io::Result<(TcpStream, SocketAddr)> {
    let socket = SocketAddrV4::new(address, port);
    let tcp_listener = match TcpListener::bind(socket) {
        Ok(tcp_listener) => tcp_listener,
        Err(error) => {

            let message = format!("Unable to bind to socket: {}", error);

            cfg_if! {
                if #[cfg(any(not(test)))] {
                    // Using unwrap since we are guaranteed to get the mutex
                    TO_UI.lock().unwrap().write(message.as_bytes())
                        .unwrap(); // Second unwrap because I am unsure of how to hand error if We
                                   // cannot send information to the pipe.
                }
            }
            panic!("{}", message)
        }
    };

    match tcp_listener.set_nonblocking(true) {
        Ok(_) => (),
        // TODO: Send message to UI we were not able to set nonblocking
        Err(error) => unimplemented!() 
    }

    tcp_listener.accept()
    
}

#[cfg(test)]
mod tests;