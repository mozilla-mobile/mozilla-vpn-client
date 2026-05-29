/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::ffi::{c_char, CStr};
use std::net::IpAddr;
use std::process::ExitCode;
use std::sync::atomic::AtomicBool;
use std::sync::Arc;

use clap::{Parser, Subcommand};

use obfuscators::factory::create_obfuscator;
use obfuscators::{Config, ObfuscationMethod};

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
}

fn parse_nonzero_port(s: &str) -> Result<u16, String> {
    let port: u16 = s.parse().map_err(|_| format!("invalid port '{s}'"))?;
    if port == 0 {
        return Err("port cannot be 0".into());
    }
    Ok(port)
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
