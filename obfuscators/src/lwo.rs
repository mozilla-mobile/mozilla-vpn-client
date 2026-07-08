/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::io;
use std::net::SocketAddr;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use tokio::net::UdpSocket;
use tokio::runtime::Runtime;

use crate::obfuscator::{Config, LwoVersion, Obfuscator};

mod v1;
mod v2;

const SHUTDOWN_POLL: Duration = Duration::from_millis(200);
const MAX_PACKET: usize = 65535;

type WgAddr = Arc<Mutex<Option<SocketAddr>>>;

pub struct LwoObfuscator {
    runtime: Option<Runtime>,
    local: Option<Arc<UdpSocket>>,
    remote: Option<Arc<UdpSocket>>,
    local_port: u16,
    socket_v4: i32,
    socket_v6: i32,
    version: LwoVersion,
    server_public_key: [u8; 32],
    public_key: [u8; 32],
}

impl LwoObfuscator {
    pub fn new(cfg: &Config) -> io::Result<Self> {
        let server = cfg.server_addr();
        let server_public_key = cfg.server_public_key.ok_or_else(|| {
            io::Error::new(
                io::ErrorKind::InvalidInput,
                "LWO requires the WireGuard server public key",
            )
        })?;
        match cfg.lwo_version {
            LwoVersion::V1 => log::info!("Using LWO v1 obfuscation"),
            LwoVersion::V2 => log::info!("Using LWO v2 obfuscation"),
        }
        // v2 obfuscates in both directions with the server's public key, so the
        // client's own key is only needed for v1 inbound deobfuscation.
        let public_key = match cfg.lwo_version {
            LwoVersion::V1 => cfg.public_key.ok_or_else(|| {
                io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "LWO v1 requires the WireGuard public key",
                )
            })?,
            LwoVersion::V2 => cfg.public_key.unwrap_or([0u8; 32]),
        };
        let runtime = tokio::runtime::Builder::new_current_thread()
            .enable_all()
            .build()?;

        let listen_port = cfg.listen_port;
        #[cfg(target_os = "linux")]
        let fwmark = cfg.fwmark;
        let (local, remote) = runtime.block_on(async {
            let local = UdpSocket::bind(("127.0.0.1", listen_port)).await?;
            let remote = match server {
                SocketAddr::V4(_) => UdpSocket::bind(("0.0.0.0", 0)).await?,
                SocketAddr::V6(_) => UdpSocket::bind(("::", 0)).await?,
            };
            // Protect the outbound socket so its packets are routed around the
            // VPN tunnel instead of back into it. Must be set before connect()
            // so route selection observes the mark.
            #[cfg(target_os = "linux")]
            if let Some(fwmark) = fwmark {
                set_fwmark(&remote, fwmark)?;
            }
            remote.connect(server).await?;
            io::Result::Ok((local, remote))
        })?;

        let local_port = local.local_addr()?.port();
        let fd = socket_fd(&remote);
        let (socket_v4, socket_v6) = match server {
            SocketAddr::V4(_) => (fd, -1),
            SocketAddr::V6(_) => (-1, fd),
        };

        Ok(Self {
            runtime: Some(runtime),
            local: Some(Arc::new(local)),
            remote: Some(Arc::new(remote)),
            local_port,
            socket_v4,
            socket_v6,
            version: cfg.lwo_version,
            server_public_key,
            public_key,
        })
    }
}

impl Obfuscator for LwoObfuscator {
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
        let Some(local) = self.local.take() else {
            return;
        };
        let Some(remote) = self.remote.take() else {
            return;
        };

        let server_public_key = self.server_public_key;
        let public_key = self.public_key;
        let version = self.version;

        runtime.block_on(async move {
            let wg_addr: WgAddr = Arc::new(Mutex::new(None));

            let (outbound, inbound) = match version {
                LwoVersion::V1 => (
                    tokio::spawn(v1::outbound(
                        Arc::clone(&local),
                        Arc::clone(&remote),
                        Arc::clone(&wg_addr),
                        server_public_key,
                    )),
                    tokio::spawn(v1::inbound(
                        Arc::clone(&local),
                        Arc::clone(&remote),
                        Arc::clone(&wg_addr),
                        public_key,
                    )),
                ),
                LwoVersion::V2 => (
                    tokio::spawn(v2::outbound(
                        Arc::clone(&local),
                        Arc::clone(&remote),
                        Arc::clone(&wg_addr),
                        server_public_key,
                    )),
                    tokio::spawn(v2::inbound(
                        Arc::clone(&local),
                        Arc::clone(&remote),
                        Arc::clone(&wg_addr),
                        server_public_key,
                    )),
                ),
            };

            let mut ticker = tokio::time::interval(SHUTDOWN_POLL);
            loop {
                ticker.tick().await;
                if shutdown.load(Ordering::Acquire) {
                    break;
                }
            }
            outbound.abort();
            inbound.abort();
            let _ = outbound.await;
            let _ = inbound.await;
        });
    }
}

#[cfg(unix)]
fn socket_fd(s: &UdpSocket) -> i32 {
    use std::os::unix::io::AsRawFd;
    s.as_raw_fd()
}

#[cfg(not(unix))]
fn socket_fd(_s: &UdpSocket) -> i32 {
    -1
}

/// Set SO_MARK on the outbound socket so the platform's routing policy keeps
/// its traffic out of the VPN tunnel (avoiding a routing loop)
#[cfg(target_os = "linux")]
fn set_fwmark(socket: &UdpSocket, fwmark: u32) -> io::Result<()> {
    use nix::sys::socket::{setsockopt, sockopt};
    setsockopt(socket, sockopt::Mark, &fwmark)
        .map_err(|e| io::Error::new(io::ErrorKind::Other, format!("failed to set SO_MARK: {e}")))
}

// WG message types, copied from gotatun
type MessageType = u8;
const HANDSHAKE_INIT: MessageType = 1;
const HANDSHAKE_RESP: MessageType = 2;
const COOKIE_REPLY: MessageType = 3;
const DATA: MessageType = 4;

const HANDSHAKE_INIT_SZ: usize = 148;
const HANDSHAKE_RESP_SZ: usize = 92;
const COOKIE_REPLY_SZ: usize = 64;
const DATA_OVERHEAD_SZ: usize = 32;
