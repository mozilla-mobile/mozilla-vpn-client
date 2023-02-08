/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::os::raw::c_char;
use std::ffi::CString;

use log::{Level, LevelFilter, Log, Metadata, Record};
use once_cell::sync::OnceCell;

// Logger implementation to integrate the Rust logs with src/shared/loghandler.cpp.
//
// This message handler should be initialized alogside the Qt message handler.
pub struct Logger {
    message_handler: extern fn(i32, *mut c_char)
}

static LOGGER: OnceCell<Logger> = OnceCell::new();

impl Logger {
    fn new(f: extern fn(i32, *mut c_char)) -> Logger {
        Logger {
            message_handler: f
        }
    }

    pub fn init(f: extern fn(i32, *mut c_char)) {
        let logger = LOGGER.get_or_init(|| Logger::new(f));
        if log::set_logger(logger).is_ok() {
            log::set_max_level(LevelFilter::Debug);
        }
    }
}

impl Log for Logger {
    fn enabled(&self, _metadata: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        // The numbers here are dictated by the order of the enum on
        // src/shared/loglevel.h
        let log_level: i32 = match record.level() {
            Level::Error => 4,
            Level::Warn => 3,
            Level::Info => 2,
            Level::Debug => 1,
            Level::Trace => 0,
        };

        if let Ok(message) = CString::new(record.args().to_string()) {
            (self.message_handler)(log_level, message.as_ptr() as *mut c_char);
        }
    }

    fn flush(&self) {}
}
