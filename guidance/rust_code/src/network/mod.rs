use std::net::{Ipv4Addr, SocketAddrV4, SocketAddr};
use std::io;
use cfg_if::cfg_if;

cfg_if! {

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
        Err(error) => panic!("unable to bind {:?}", error)
        // TODO: Send message to UI
    };

    match tcp_listener.set_nonblocking(true) {
        Ok(_) => (),
        Err(error) => unimplemented!() // TODO: Send message to UI we were not able 
    }

    tcp_listener.accept()
    
}

#[cfg(test)]
mod tests;