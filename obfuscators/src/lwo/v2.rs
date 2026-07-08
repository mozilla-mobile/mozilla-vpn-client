/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


/// The LWO v2 spec also jitters selected WireGuard timers
/// (keepalive, rekey, retransmit). That is intentionally not done
/// here: this obfuscator is a userspace UDP proxy and does not drive
/// the WireGuard state machine, so it cannot adjust those timers.
/// Only the packet obfuscation is applied.

use std::sync::Arc;

use rand::rngs::OsRng;
use rand::RngCore;
use tokio::net::UdpSocket;

use super::{
    MessageType, WgAddr, DATA, HANDSHAKE_INIT, HANDSHAKE_INIT_SZ, HANDSHAKE_RESP, HANDSHAKE_RESP_SZ,
    MAX_PACKET,
};

pub async fn outbound(local: Arc<UdpSocket>, remote: Arc<UdpSocket>, wg_addr: WgAddr, key: [u8; 32]) {
    let mut buf = vec![0u8; MAX_PACKET];
    let mut rng = OsRng;
    loop {
        let Ok((n, src)) = local.recv_from(&mut buf).await else {
            break;
        };
        *wg_addr.lock().unwrap() = Some(src);
        let send_len = obfuscate(&mut rng, &mut buf, n, &key);
        let _ = remote.send(&buf[..send_len]).await;
    }
}

pub async fn inbound(local: Arc<UdpSocket>, remote: Arc<UdpSocket>, wg_addr: WgAddr, key: [u8; 32]) {
    let mut buf = vec![0u8; MAX_PACKET];
    loop {
        let Ok(n) = remote.recv(&mut buf).await else {
            break;
        };
        let Some(addr) = *wg_addr.lock().unwrap() else {
            continue;
        };
        if let Some(len) = deobfuscate(&mut buf, n, &key) {
            let _ = local.send_to(&buf[..len], addr).await;
        }
    }
}

/// Leading bytes obfuscated for a data packet in v2 (v1 obfuscates 32)
const DATA_PROTECTED_SZ_V2: usize = 16;
/// A v2 data packet is valid only when its UDP payload is at least this long
const DATA_MIN_SZ_V2: usize = 32;
/// Maximum random padding appended to a v2 handshake (range is 1..256)
const MAX_HANDSHAKE_PADDING: u32 = 256;
/// Marker bits (byte 1): bit 7 = 0, bit 6 = 1
const V2_MARKER_MASK: u8 = 0xc0;
const V2_MARKER: u8 = 0x40;

/// Length of the leading area LWO v2 obfuscates for each WireGuard message
/// type. Returns `None` for types not covered by v2 (cookie reply, unknown)
fn protected_area(msg_type: MessageType) -> Option<usize> {
    match msg_type {
        HANDSHAKE_INIT => Some(HANDSHAKE_INIT_SZ),
        HANDSHAKE_RESP => Some(HANDSHAKE_RESP_SZ),
        DATA => Some(DATA_PROTECTED_SZ_V2),
        _ => None,
    }
}

/// Whether the on-wire length is valid for a recovered v2 message type
fn valid_len(msg_type: MessageType, n: usize) -> bool {
    let pad = MAX_HANDSHAKE_PADDING as usize;
    match msg_type {
        HANDSHAKE_INIT => (HANDSHAKE_INIT_SZ..=HANDSHAKE_INIT_SZ + pad).contains(&n),
        HANDSHAKE_RESP => (HANDSHAKE_RESP_SZ..=HANDSHAKE_RESP_SZ + pad).contains(&n),
        DATA => n >= DATA_MIN_SZ_V2,
        _ => false,
    }
}

/// Obfuscate a plaintext WireGuard packet in place for LWO v2.
///
/// `buf` holds the packet in `buf[..n]` and must have spare capacity for
/// appended handshake padding. Returns the number of bytes to send, which may
/// exceed `n` for padded handshakes. Cookie replies and packets that are not
/// recognized WireGuard messages are left untouched (returned length `n`)
pub fn obfuscate(rng: &mut impl RngCore, buf: &mut [u8], n: usize, key: &[u8; 32]) -> usize {
    if n == 0 {
        return 0;
    }
    let msg_type = buf[0];
    let Some(protected) = protected_area(msg_type) else {
        // Cookie replies and anything unrecognized  must be sent unchanged
        return n;
    };
    if n < protected {
        return n;
    }

    // Handshake initiations and responses get 1..256 random padding bytes
    // appended before obfuscation. The padding itself is not obfuscated, but
    // the padded length is used by length mix
    let padded_len = match msg_type {
        HANDSHAKE_INIT | HANDSHAKE_RESP => {
            let pad = 1 + (rng.next_u32() % MAX_HANDSHAKE_PADDING) as usize;
            let new_len = n + pad;
            if new_len <= buf.len() {
                rng.fill_bytes(&mut buf[n..new_len]);
                new_len
            } else {
                n
            }
        }
        _ => n,
    };

    let len_mix = padded_len as u8;
    apply_len_mixed_xor(&mut buf[..protected], len_mix, key);
    buf[1] = (key[0].wrapping_add(len_mix) & 0x3f) | V2_MARKER;

    padded_len
}

pub fn deobfuscate(buf: &mut [u8], n: usize, key: &[u8; 32]) -> Option<usize> {
    // A packet is LWO v2 only when the marker bits in byte 1 are set
    if n < 2 || buf[1] & V2_MARKER_MASK != V2_MARKER {
        return Some(n);
    }
    // is LWO v2 but is too short, drop it
    if n < 4 {
        return None;
    }

    let len_mix = n as u8;

    // Validate bytes 1, 2 and 3 against their expected obfuscated values
    let expect1 = (key[0].wrapping_add(len_mix) & 0x3f) | V2_MARKER;
    let expect2 = key[2].wrapping_add(len_mix).wrapping_add(2);
    let expect3 = key[3].wrapping_add(len_mix).wrapping_add(3);
    if buf[1] != expect1 || buf[2] != expect2 || buf[3] != expect3 {
        return None;
    }

    // Recover the WireGuard message type. Unknown types and cookie replies
    // (type 3, excluded from LWO v2) are dropped
    let msg_type = buf[0] ^ key[0].wrapping_add(len_mix);
    let protected = protected_area(msg_type)?;
    if !valid_len(msg_type, n) {
        return None;
    }

    // Same XOR pass as obfuscation restores the header, then byte 1 is cleared
    apply_len_mixed_xor(&mut buf[..protected], len_mix, key);
    buf[1] = 0x00;

    // Non-data packets carry padding past the protected area; trim it before
    // handing the packet to WireGuard. Data packets keep their full length
    if msg_type != DATA && n > protected {
        Some(protected)
    } else {
        Some(n)
    }
}

fn apply_len_mixed_xor(buf: &mut [u8], len_mix: u8, key: &[u8; 32]) {
    for (i, b) in buf.iter_mut().enumerate() {
        *b ^= key[i % 32].wrapping_add(len_mix).wrapping_add(i as u8);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lwo::{COOKIE_REPLY, COOKIE_REPLY_SZ, MAX_PACKET};
    use rand::rngs::OsRng;

    fn key() -> [u8; 32] {
        let mut k = [0u8; 32];
        for (i, b) in k.iter_mut().enumerate() {
            *b = (i as u8).wrapping_mul(7).wrapping_add(3);
        }
        k
    }

    /// A v2 handshake initiation round-trips: obfuscate pads + marks it, and
    /// deobfuscate validates, restores the header and trims the padding
    #[test]
    fn handshake_roundtrip() {
        let key = key();
        let mut original = vec![0u8; HANDSHAKE_INIT_SZ];
        original[0] = HANDSHAKE_INIT;
        for (i, b) in original.iter_mut().enumerate().skip(4) {
            *b = (i as u8) ^ 0xa5;
        }

        let mut buf = vec![0u8; MAX_PACKET];
        buf[..HANDSHAKE_INIT_SZ].copy_from_slice(&original);

        let sent = obfuscate(&mut OsRng, &mut buf, HANDSHAKE_INIT_SZ, &key);
        // Padding was appended and the marker set.
        assert!(sent > HANDSHAKE_INIT_SZ && sent <= HANDSHAKE_INIT_SZ + 256);
        assert_eq!(buf[1] & V2_MARKER_MASK, V2_MARKER);

        let out = deobfuscate(&mut buf, sent, &key).expect("must forward");
        assert_eq!(out, HANDSHAKE_INIT_SZ, "padding trimmed");
        assert_eq!(&buf[..HANDSHAKE_INIT_SZ], &original[..]);
    }

    /// A v2 data packet round-trips without padding or trimming
    #[test]
    fn data_roundtrip() {
        let key = key();
        let len = 128;
        let mut original = vec![0u8; len];
        original[0] = DATA;
        for (i, b) in original.iter_mut().enumerate().skip(4) {
            *b = (i as u8).wrapping_mul(3);
        }

        let mut buf = vec![0u8; MAX_PACKET];
        buf[..len].copy_from_slice(&original);

        let sent = obfuscate(&mut OsRng, &mut buf, len, &key);
        assert_eq!(sent, len, "data packets are not padded");
        assert_eq!(buf[1] & V2_MARKER_MASK, V2_MARKER);
        assert_ne!(&buf[..DATA_PROTECTED_SZ_V2], &original[..DATA_PROTECTED_SZ_V2]);

        let out = deobfuscate(&mut buf, sent, &key).expect("must forward");
        assert_eq!(out, len);
        assert_eq!(&buf[..len], &original[..]);
    }

    /// Cookie replies are not LWO v2 packets: never obfuscated on send, and
    /// passed through unchanged on receive
    #[test]
    fn cookie_reply_passthrough() {
        let key = key();
        let mut buf = vec![0u8; MAX_PACKET];
        buf[0] = COOKIE_REPLY;
        let before = buf[..COOKIE_REPLY_SZ].to_vec();

        let sent = obfuscate(&mut OsRng, &mut buf, COOKIE_REPLY_SZ, &key);
        assert_eq!(sent, COOKIE_REPLY_SZ);
        assert_eq!(&buf[..COOKIE_REPLY_SZ], &before[..], "cookie reply untouched");

        // A plain packet whose byte 1 lacks the marker passes through on receive.
        let out = deobfuscate(&mut buf, COOKIE_REPLY_SZ, &key).expect("pass through");
        assert_eq!(out, COOKIE_REPLY_SZ);
    }

    /// A packet that claims LWO v2 but fails byte 1/2/3 validation is dropped
    #[test]
    fn invalid_marker_dropped() {
        let key = key();
        let mut buf = vec![0u8; 200];
        buf[0] = 0x11;
        buf[1] = V2_MARKER; // claims v2 but bytes 2/3 won't validate
        buf[2] = 0x00;
        buf[3] = 0x00;
        assert_eq!(deobfuscate(&mut buf, 200, &key), None);
    }
}
