/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::ffi::{c_char, CStr};
use std::net::IpAddr;
use std::process::ExitCode;
use std::sync::atomic::AtomicBool;
use std::sync::Arc;

use base64::Engine;
use clap::{Parser, Subcommand, ValueEnum};

use obfuscators::factory::create_obfuscator;
use obfuscators::{Config, LwoVersion, ObfuscationMethod};

#[derive(Parser)]
#[command(
    name = "mozillavpn-obfuscator",
    about = "Run a Mozilla VPN obfuscator as a standalone local proxy",
    version
)]
struct Cli {
    #[command(subcommand)]
    obfuscator: Obfuscator,
}

#[derive(Subcommand)]
enum Obfuscator {
    /// Tunnel WireGuard UDP through a TCP stream.
    UdpOverTcp {
        /// Server IPv4 or IPv6 address.
        #[arg(short, long)]
        server: IpAddr,
        /// Server TCP port.
        #[arg(short, long, value_parser = parse_nonzero_port)]
        port: u16,
        /// Local UDP port to listen on (127.0.0.1). Defaults to OS-assigned.
        #[arg(short, long, value_parser = parse_nonzero_port)]
        listen_port: Option<u16>,
        /// Fwmark
        #[arg(long)]
        fwmark: Option<u32>,
    },
    /// Obfuscate WireGuard UDP with Lightweight Obfuscation (LWO).
    Lwo {
        /// Server IPv4 or IPv6 address.
        #[arg(short, long)]
        server: IpAddr,
        /// Server UDP port.
        #[arg(short, long, value_parser = parse_nonzero_port)]
        port: u16,
        /// Local UDP port to listen on (127.0.0.1). Defaults to OS-assigned.
        #[arg(short, long, value_parser = parse_nonzero_port)]
        listen_port: Option<u16>,
        /// Server WireGuard public key (base64). Required by LWO.
        #[arg(long, value_parser = parse_wg_key)]
        server_public_key: [u8; 32],
        /// Client WireGuard public key (base64). Required by LWO v1.
        #[arg(long, value_parser = parse_wg_key)]
        public_key: Option<[u8; 32]>,
        /// LWO protocol version.
        #[arg(long, value_enum, default_value_t = LwoVersionArg::V1)]
        lwo_version: LwoVersionArg,
        /// Fwmark
        #[arg(long)]
        fwmark: Option<u32>,
    },
}

/// CLI mirror of `obfuscators::LwoVersion`
#[derive(Copy, Clone, ValueEnum)]
enum LwoVersionArg {
    V1,
    V2,
}

impl From<LwoVersionArg> for LwoVersion {
    fn from(v: LwoVersionArg) -> Self {
        match v {
            LwoVersionArg::V1 => LwoVersion::V1,
            LwoVersionArg::V2 => LwoVersion::V2,
        }
    }
}

fn parse_nonzero_port(s: &str) -> Result<u16, String> {
    let port: u16 = s.parse().map_err(|_| format!("invalid port '{s}'"))?;
    if port == 0 {
        return Err("port cannot be 0".into());
    }
    Ok(port)
}

fn parse_wg_key(s: &str) -> Result<[u8; 32], String> {
    let decoded = base64::engine::general_purpose::STANDARD
        .decode(s)
        .map_err(|e| format!("invalid base64 key: {e}"))?;
    decoded
        .try_into()
        .map_err(|v: Vec<u8>| format!("WireGuard key must be 32 bytes, got {}", v.len()))
}

fn main() -> ExitCode {
    let cli = Cli::parse();
    obfuscators::obfuscators_set_log_handler(log_to_stderr);

    let cfg = match cli.obfuscator {
        Obfuscator::UdpOverTcp { server, port, listen_port, fwmark } => {
            let (server_ipv4, server_ipv6) = match server {
                IpAddr::V4(v4) => (Some(v4), None),
                IpAddr::V6(v6) => (None, Some(v6)),
            };
            Config {
                method: ObfuscationMethod::UdpOverTcp,
                public_key: None,
                server_ipv4,
                server_ipv6,
                server_port: port,
                listen_port: listen_port.unwrap_or(0),
                server_public_key: None,
                lwo_version: LwoVersion::V1,
                fwmark,
            }
        }
        Obfuscator::Lwo {
            server,
            port,
            listen_port,
            server_public_key,
            public_key,
            lwo_version,
            fwmark,
        } => {
            let (server_ipv4, server_ipv6) = match server {
                IpAddr::V4(v4) => (Some(v4), None),
                IpAddr::V6(v6) => (None, Some(v6)),
            };
            Config {
                method: ObfuscationMethod::Lwo,
                public_key,
                server_ipv4,
                server_ipv6,
                server_port: port,
                listen_port: listen_port.unwrap_or(0),
                server_public_key: Some(server_public_key),
                lwo_version: lwo_version.into(),
                fwmark,
            }
        }
    };

    let mut obf = match create_obfuscator(&cfg) {
        Ok(o) => o,
        Err(e) => {
            eprintln!("error: failed to start obfuscator: {e}");
            return ExitCode::FAILURE;
        }
    };

    println!("listening on 127.0.0.1:{}", obf.local_port());
    obf.run(Arc::new(AtomicBool::new(false)));
    ExitCode::SUCCESS
}

extern "C" fn log_to_stderr(level: i32, msg: *mut c_char) {
    if msg.is_null() {
        return;
    }
    let tag = match level {
        0 => "TRACE",
        1 => "DEBUG",
        2 => "INFO",
        3 => "WARN",
        4 => "ERROR",
        _ => "LOG",
    };
    let s = unsafe { CStr::from_ptr(msg) }.to_string_lossy();
    eprintln!("[{tag}] {s}");
}
