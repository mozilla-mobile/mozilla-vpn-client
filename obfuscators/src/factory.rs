/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::io;

use crate::obfuscator::{Config, Obfuscator, ObfuscationMethod};

use crate::lwo::LwoObfuscator;
use crate::udp_over_tcp::UdpOverTcpObfuscator;

pub fn create_obfuscator(cfg: &Config) -> io::Result<Box<dyn Obfuscator>> {
    log::info!("Creating obfuscator for config: {cfg:#?}");
    match cfg.method {
        ObfuscationMethod::UdpOverTcp => Ok(Box::new(UdpOverTcpObfuscator::new(cfg)?)),
        ObfuscationMethod::Lwo => Ok(Box::new(LwoObfuscator::new(cfg)?)),

        // No obfuscation should raise an error as the daemon should skip start the obfuscator in this case
        m => Err(io::Error::new(
            io::ErrorKind::InvalidInput,
            format!("obfuscation method not implemented: {m:?}"),
        )),
    }
}
