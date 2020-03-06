use std::net::{Ipv4Addr, SocketAddrV4, TcpListener};
use mockall::*;
use cfg_if::cfg_if;

const GUIDANCE_IP: Ipv4Addr = Ipv4Addr::new(192, 168, 1, 1);
const CONTROL_PORT: u16 = 31401;

mock! {
    pub TcpListener {
        fn bind<A: ToSocketAddrs + 'static>(addr: A) -> Result<TcpListener>;
    }
}

cfg_if! {
    if #[cfg(test)] {
        use MockTcpListener as TcpListener;
    }
}

// This function is not finished yet
pub(self) fn establish_connection(address: Ipv4Addr, port: u16) {
    let socket = SocketAddrV4::new(address, port);
    let tcp_listener = TcpListener::bind(socket); // invocation of the method I am trying to mock
}

#[cfg(test)]
mod tests;