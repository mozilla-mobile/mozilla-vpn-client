/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::os::raw::c_char;
use std::ptr;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::thread::{self, JoinHandle};
use logger::Logger;

mod factory;
mod logger;
#[allow(dead_code)]
mod obfuscator;
mod udp_over_tcp;

pub use obfuscator::{Obfuscator, ObfuscatorConfig, Config};

/// Opaque handle held by C++ caller.
/// Owns the runner thread and the shutdown flag dropping it stops the obfuscator.
pub struct ObfuscatorHandle {
    shutdown: Arc<AtomicBool>,
    thread: Option<JoinHandle<()>>,
    local_port: u16,
    socket_v4: i32,
    socket_v6: i32,
}

impl Drop for ObfuscatorHandle {
    fn drop(&mut self) {
        self.shutdown.store(true, Ordering::Release);
        if let Some(t) = self.thread.take() {
            let _ = t.join();
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn obfuscator_start(
    cfg: *const ObfuscatorConfig,
) -> *mut ObfuscatorHandle {
    let Some(cfg) = Config::from_c(cfg) else {
        return ptr::null_mut();
    };

    let Ok(mut obf) = factory::create_obfuscator(&cfg) else {
        return ptr::null_mut();
    };

    let local_port = obf.local_port();
    let socket_v4 = obf.socket_v4();
    let socket_v6 = obf.socket_v6();

    let shutdown = Arc::new(AtomicBool::new(false));
    let shutdown_run = Arc::clone(&shutdown);
    let Ok(thread) = thread::Builder::new()
        .name("obf-runner".into())
        .spawn(move || obf.run(shutdown_run))
    else {
        return ptr::null_mut();
    };

    Box::into_raw(Box::new(ObfuscatorHandle {
        shutdown,
        thread: Some(thread),
        local_port,
        socket_v4,
        socket_v6,
    }))
}

#[no_mangle]
pub unsafe extern "C" fn obfuscator_local_port(
    handle: *const ObfuscatorHandle,
) -> u16 {
    if handle.is_null() {
        return 0;
    }
    (*handle).local_port
}

#[no_mangle]
pub unsafe extern "C" fn obfuscator_socket_v4(
    handle: *const ObfuscatorHandle,
) -> i32 {
    if handle.is_null() {
        return -1;
    }
    (*handle).socket_v4
}


#[no_mangle]
pub unsafe extern "C" fn obfuscator_socket_v6(
    handle: *const ObfuscatorHandle,
) -> i32 {
    if handle.is_null() {
        return -1;
    }
    (*handle).socket_v6
}

#[no_mangle]
pub unsafe extern "C" fn obfuscator_stop(handle: *mut ObfuscatorHandle) {
    if handle.is_null() {
        return;
    }
    drop(Box::from_raw(handle));
}

#[no_mangle]
pub extern "C" fn obfuscators_set_log_handler(message_handler: extern "C" fn(i32, *mut c_char)){
    Logger::init(message_handler);
}
