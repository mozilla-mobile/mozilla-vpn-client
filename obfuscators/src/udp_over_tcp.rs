/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::io;
use std::net::SocketAddr;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::Duration;

use tokio::runtime::Runtime;
use udp_over_tcp::{TcpOptions, Udp2Tcp};

use crate::obfuscator::{Config, Obfuscator};

const SHUTDOWN_POLL: Duration = Duration::from_millis(200);

/// Tunnels UDP traffic through a TCP stream using mullvad/udp-over-tcp.
/// The TCP socket is what callers must mark/protect.

pub struct UdpOverTcpObfuscator {
    runtime: Option<Runtime>,
    inner: Option<Udp2Tcp>,
    local_port: u16,
    socket_v4: i32,
    socket_v6: i32,
}

impl UdpOverTcpObfuscator {
    pub fn new(cfg: &Config) -> io::Result<Self> {
        let server = cfg.server_addr();
        let runtime = tokio::runtime::Builder::new_current_thread()
            .enable_all()
            .build()?;

        let listen_addr: SocketAddr = ([127, 0, 0, 1], cfg.listen_port).into();
        let mut tcp_options = TcpOptions::default();
        tcp_options.nodelay = true;
        #[cfg(target_os = "linux")]
        {
            tcp_options.fwmark = cfg.fwmark;
        }
        let inner = runtime
            .block_on(Udp2Tcp::new(listen_addr, server, tcp_options))
            .map_err(|e| io::Error::new(io::ErrorKind::Other, e.to_string()))?;

        let local_port = inner.local_udp_addr()?.port();
        let (socket_v4, socket_v6) = tcp_fds(&inner, server);

        Ok(Self {
            runtime: Some(runtime),
            inner: Some(inner),
            local_port,
            socket_v4,
            socket_v6,
        })
    }
}

impl Obfuscator for UdpOverTcpObfuscator {
    fn local_port(&self) -> u16 {
        self.local_port
    }
    fn socket_v4(&self) -> i32 {
        self.socket_v4
    }
    fn socket_v6(&self) -> i32 {
        self.socket_v6
    }

    fn run(&mut self, shutdown: Arc<AtomicBool>) {
        let Some(runtime) = self.runtime.take() else {
            return;
        };
        let Some(inner) = self.inner.take() else {
            return;
        };

        runtime.block_on(async move {
            let run_fut = inner.run();
            tokio::pin!(run_fut);
            loop {
                tokio::select! {
                    res = &mut run_fut => {
                        let _ = res;
                        return;
                    }
                    _ = tokio::time::sleep(SHUTDOWN_POLL) => {
                        if shutdown.load(Ordering::Acquire) {
                            return;
                        }
                    }
                }
            }
        });
    }
}

#[cfg(unix)]
fn tcp_fds(inner: &Udp2Tcp, server: SocketAddr) -> (i32, i32) {
    let fd = inner.remote_tcp_fd();
    match server {
        SocketAddr::V4(_) => (fd, -1),
        SocketAddr::V6(_) => (-1, fd),
    }
}

#[cfg(not(unix))]
fn tcp_fds(_inner: &Udp2Tcp, _server: SocketAddr) -> (i32, i32) {
    (-1, -1)
}
