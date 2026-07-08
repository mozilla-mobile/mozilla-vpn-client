/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//! LWO v1 packet transformation.

use std::sync::Arc;

use rand::rngs::OsRng;
use rand::RngCore;
use tokio::net::UdpSocket;

use super::{
    WgAddr, COOKIE_REPLY, COOKIE_REPLY_SZ, DATA, DATA_OVERHEAD_SZ, HANDSHAKE_INIT,
    HANDSHAKE_INIT_SZ, HANDSHAKE_RESP, HANDSHAKE_RESP_SZ, MAX_PACKET,
};

/// Bit to set in the second byte of the WG header to enable LWO
const OBFUSCATION_BIT: u8 = 0b10000000;

pub async fn outbound(
    local: Arc<UdpSocket>,
    remote: Arc<UdpSocket>,
    wg_addr: WgAddr,
    key: [u8; 32],
) {
    let mut buf = vec![0u8; MAX_PACKET];
    let mut rng = OsRng;
    loop {
        let Ok((n, src)) = local.recv_from(&mut buf).await else {
            break;
        };
        *wg_addr.lock().unwrap() = Some(src);
        obfuscate(&mut rng, &mut buf[..n], &key);
        let _ = remote.send(&buf[..n]).await;
    }
}

pub async fn inbound(
    local: Arc<UdpSocket>,
    remote: Arc<UdpSocket>,
    wg_addr: WgAddr,
    key: [u8; 32],
) {
    let mut buf = vec![0u8; MAX_PACKET];
    loop {
        let Ok(n) = remote.recv(&mut buf).await else {
            break;
        };
        let Some(addr) = *wg_addr.lock().unwrap() else {
            continue;
        };
        deobfuscate(&mut buf[..n], &key);
        let _ = local.send_to(&buf[..n], addr).await;
    }
}

pub fn obfuscate(rng: &mut impl RngCore, packet: &mut [u8], key: &[u8; 32]) {
    log::error!("Obfuscating packet of size {}", packet.len());

    let Some(header) = resolve_packet_header(packet, 0) else {
        return;
    };

    // Encrypt the header region with a repeating 32-byte key
    apply_cyclic_xor(header, key);

    // Generate 7 random bits and set the MSB to mark the packet
    header[1] = (rng.next_u32() as u8 & 0x7F) | OBFUSCATION_BIT;
}

pub fn deobfuscate(packet: &mut [u8], key: &[u8; 32]) {
    log::info!("Deobfuscating packet of size {}", packet.len());

    let Some(header) = resolve_packet_header(packet, key[0]) else {
        return;
    };

    #[cfg(debug_assertions)]
    if !has_obfuscation_flag(header[1]) {
        log::error!("Received non-obfuscated packet from relay");
        return;
    }

    // XOR is symmetric, so the same operation reverses the encryption
    apply_cyclic_xor(header, key);
    header[1] = 0;
}

#[cfg(debug_assertions)]
const fn has_obfuscation_flag(byte: u8) -> bool {
    byte & OBFUSCATION_BIT != 0
}

/// Resolves the correct header slice by decoding the message type via XOR
fn resolve_packet_header<'a>(pkt: &'a mut [u8], seed: u8) -> Option<&'a mut [u8]> {
    let raw_type = *pkt.first()?;
    let decoded_type = raw_type ^ seed;

    let expected_len = match decoded_type {
        HANDSHAKE_INIT => HANDSHAKE_INIT_SZ,
        HANDSHAKE_RESP => HANDSHAKE_RESP_SZ,
        COOKIE_REPLY => COOKIE_REPLY_SZ,
        DATA => DATA_OVERHEAD_SZ,
        _ => return None,
    };

    pkt.get_mut(..expected_len)
}

fn apply_cyclic_xor(buf: &mut [u8], key: &[u8; 32]) {
    for (b, k) in buf.iter_mut().zip(key.iter().cycle()) {
        *b ^= *k;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use rand::rngs::OsRng;

    fn key() -> [u8; 32] {
        let mut k = [0u8; 32];
        for (i, b) in k.iter_mut().enumerate() {
            *b = (i as u8).wrapping_mul(7).wrapping_add(3);
        }
        k
    }

    /// Build a WireGuard-looking packet: `type` in byte 0, reserved byte 1 = 0
    /// (as WireGuard sends it), and deterministic filler afterwards.
    fn packet(msg_type: u8, len: usize) -> Vec<u8> {
        let mut p = vec![0u8; len];
        p[0] = msg_type;
        for (i, b) in p.iter_mut().enumerate().skip(2) {
            *b = (i as u8) ^ 0x5a;
        }
        p
    }

    /// A v1 handshake initiation round-trips: obfuscate scrambles the header and
    /// sets the marker bit; deobfuscate restores the original bytes.
    #[test]
    fn v1_handshake_roundtrip() {
        let key = key();
        let original = packet(HANDSHAKE_INIT, HANDSHAKE_INIT_SZ);
        let mut buf = original.clone();

        obfuscate(&mut OsRng, &mut buf, &key);
        assert_ne!(
            &buf[..HANDSHAKE_INIT_SZ],
            &original[..],
            "header obfuscated"
        );
        assert_ne!(buf[1] & OBFUSCATION_BIT, 0, "marker bit set");

        deobfuscate(&mut buf, &key);
        assert_eq!(buf, original, "header restored");
    }

    /// A v1 data packet round-trips, and the payload past the obfuscated header
    /// is left untouched in both directions.
    #[test]
    fn v1_data_roundtrip() {
        let key = key();
        let original = packet(DATA, 128);
        let mut buf = original.clone();

        obfuscate(&mut OsRng, &mut buf, &key);
        assert_ne!(&buf[..DATA_OVERHEAD_SZ], &original[..DATA_OVERHEAD_SZ]);
        assert_eq!(
            &buf[DATA_OVERHEAD_SZ..],
            &original[DATA_OVERHEAD_SZ..],
            "payload past the header is untouched"
        );

        deobfuscate(&mut buf, &key);
        assert_eq!(buf, original);
    }

    /// Packets whose type is not a known WireGuard message are left untouched.
    #[test]
    fn v1_unknown_type_untouched() {
        let key = key();
        let original = packet(0x77, 148);
        let mut buf = original.clone();

        obfuscate(&mut OsRng, &mut buf, &key);
        assert_eq!(buf, original, "unknown type not obfuscated");
    }

    /// `header_mut` selects the header length from the (unmasked) message type.
    #[test]
    fn resolve_packet_header_sizes() {
        let sizes = [
            (HANDSHAKE_INIT, HANDSHAKE_INIT_SZ),
            (HANDSHAKE_RESP, HANDSHAKE_RESP_SZ),
            (COOKIE_REPLY, COOKIE_REPLY_SZ),
            (DATA, DATA_OVERHEAD_SZ),
        ];
        for (ty, sz) in sizes {
            let mut buf = vec![0u8; 256];
            buf[0] = ty;
            assert_eq!(
                resolve_packet_header(&mut buf, 0).map(|h| h.len()),
                Some(sz)
            );
        }

        let mut buf = vec![0u8; 256];
        buf[0] = 0x77;
        assert!(resolve_packet_header(&mut buf, 0).is_none(), "unknown type");
    }
}
