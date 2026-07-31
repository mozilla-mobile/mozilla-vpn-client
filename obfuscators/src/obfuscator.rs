/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::sync::atomic::AtomicBool;
use std::sync::Arc;

use std::ffi::{c_char, CStr};
use std::net::{IpAddr, Ipv4Addr, Ipv6Addr, SocketAddr};
use std::str::FromStr;

use base64::engine::general_purpose::STANDARD as BASE64;
use base64::Engine;

pub const WG_KEY_LEN: usize = 32;


/// An obfuscator is a self-contained proxy. Implementations bind their own
/// sockets in their constructor, expose the bound port + outbound socket FDs.
pub trait Obfuscator: Send {
    /// Local UDP port the obfuscator is listening on (always on 127.0.0.1).
    /// Callers rewrite the WireGuard peer endpoint to this port.
    fn local_port(&self) -> u16;

    /// Outbound socket FD for IPv4 traffic, or -1 if not used.
    /// The platform layer marks/protects this.
    fn socket_v4(&self) -> i32 {
        -1
    }

    /// Outbound socket FD for IPv6 traffic, or -1 if not used.
    fn socket_v6(&self) -> i32 {
        -1
    }

    fn run(&mut self, shutdown: Arc<AtomicBool>);
}

// Obfuscation methods enum, remember to keep in sync with the ones in settingsholder.h and models/server.h
#[repr(u32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]

pub enum ObfuscationMethod {
    NoObfuscation = 0,
    Lwo = 1,
    Masque = 2,
    UdpOverTcp = 3,
    Shadowsocks = 4,
}

impl ObfuscationMethod {
    pub fn from_u32(v: u32) -> Option<Self> {
        match v {
            0 => Some(Self::NoObfuscation),
            1 => Some(Self::Lwo),
            2 => Some(Self::Masque),
            3 => Some(Self::UdpOverTcp),
            4 => Some(Self::Shadowsocks),
            _ => None,
        }
    }
}

/// C-ABI view: what the JNA caller actually fills in and passes across the FFI.
#[repr(C)]
pub struct ObfuscatorConfig {
    pub obfuscation_method: u32,
    pub server_ipv4_addr_in: *const c_char,
    pub server_ipv6_addr_in: *const c_char,
    pub server_port: u16,
    /// Local UDP port the obfuscator should listen on (127.0.0.1).
    /// 0 = let the OS pick a free port.
    pub listen_port: u16,
    // Wireguard keys required by LWO
    pub server_public_key: *const c_char,
    pub public_key: *const c_char,
    #[cfg(target_os = "linux")]
    pub fwmark: u32,
}

/// Safe Rust mirror used inside the crate.
#[derive(Debug, Clone)]
pub struct Config {
    pub method: ObfuscationMethod,
    pub public_key: Option<[u8; WG_KEY_LEN]>,
    pub server_ipv4: Option<Ipv4Addr>,
    pub server_ipv6: Option<Ipv6Addr>,
    pub server_port: u16,
    pub listen_port: u16,
    pub server_public_key: Option<[u8; WG_KEY_LEN]>,
    #[cfg(target_os = "linux")]
    pub fwmark: Option<u32>,
}

impl Config {
    /// Convert a C struct from C++ into a safe Rust value.
    /// Returns None in case of error: obfuscation method is unknown, the port is zero, or both addresses are missing / unparseable.
    pub unsafe fn from_c(cfg: *const ObfuscatorConfig) -> Option<Self> {
        if cfg.is_null() {
            return None;
        }
        let cfg = &*cfg;
        let method = ObfuscationMethod::from_u32(cfg.obfuscation_method)?;
        if cfg.server_port == 0 {
            return None;
        }
        let server_ipv4 = parse_addr::<Ipv4Addr>(cfg.server_ipv4_addr_in);
        let server_ipv6 = parse_addr::<Ipv6Addr>(cfg.server_ipv6_addr_in);
        if server_ipv4.is_none() && server_ipv6.is_none() {
            return None;
        }
        let public_key = parse_wg_key(cfg.public_key);
        let server_public_key = parse_wg_key(cfg.server_public_key);

        Some(Self {
            method,
            public_key,
            server_ipv4,
            server_ipv6,
            server_port: cfg.server_port,
            listen_port: cfg.listen_port,
            server_public_key,
            #[cfg(target_os = "linux")]
            fwmark: if cfg.fwmark == 0 { None } else { Some(cfg.fwmark) },
        })
    }

    pub fn server_addr(&self) -> SocketAddr {
        let ip: IpAddr = match (self.server_ipv4, self.server_ipv6) {
            (Some(v4), _) => v4.into(),
            (None, Some(v6)) => v6.into(),
            (None, None) => unreachable!("from_c rejects configs with no address"),
        };
        SocketAddr::new(ip, self.server_port)
    }
}

unsafe fn parse_addr<T: FromStr>(p: *const c_char) -> Option<T> {
    if p.is_null() {
        return None;
    }
    let s = CStr::from_ptr(p).to_str().ok()?;
    if s.is_empty() {
        return None;
    }
    T::from_str(s).ok()
}

unsafe fn parse_wg_key(p: *const c_char) -> Option<[u8; WG_KEY_LEN]> {
    if p.is_null() {
        return None;
    }
    let s = CStr::from_ptr(p).to_str().ok()?;
    if s.is_empty() {
        return None;
    }
    let decoded = BASE64.decode(s).ok()?;
    decoded.try_into().ok()
}
