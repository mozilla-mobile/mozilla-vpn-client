/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::env;
use std::os::raw::c_char;

use ffi_support::FfiStr;
use glean::{ClientInfoMetrics, Configuration};

use ffi::helpers::FallibleToString;
use metrics::__generated_pings::register_pings;
use once_cell::sync::Lazy;
use std::sync::Mutex;
use std::sync::Arc;
use uploader::VPNPingUploader;
use logger::Logger;

// Make internal Glean symbols public for mobile SDK consumption.
pub use glean_core;

mod ffi;
mod metrics;
mod uploader;
mod logger;

const GLEAN_APPLICATION_ID: &str = "mozillavpn";

static GLOBAL_PING_FILTER_LIST : Lazy<Arc<Mutex<Vec<String>>>> = Lazy::new(|| Arc::new(Mutex::new(Vec::new())));


#[no_mangle]
pub extern "C" fn glean_register_log_handler(message_handler: extern fn(i32, *mut c_char)) {
    Logger::init(message_handler);
}

#[no_mangle]
pub extern "C" fn glean_initialize(is_telemetry_enabled: bool, data_path: FfiStr, channel: FfiStr, locale: FfiStr) {
    let cfg = Configuration {
        data_path: data_path
            .to_string_fallible()
            .expect("Invalid Glean data path.")
            .into(),
        application_id: GLEAN_APPLICATION_ID.into(),
        upload_enabled: is_telemetry_enabled,
        // Default is 500
        max_events: None,
        // Whether Glean should delay persistence of data from metrics with ping lifetime
        delay_ping_lifetime_io: false,
        // Default is "https://incoming.telemetry.mozilla.org"
        server_endpoint: None,
        // Use the Glean provided one once https://bugzilla.mozilla.org/show_bug.cgi?id=1675468 is resolved
        uploader: Some(Box::new(VPNPingUploader::new(GLOBAL_PING_FILTER_LIST.clone()))),
        // Whether Glean should schedule “metrics” pings for you
        use_core_mps: true,
        trim_data_to_registered_pings: false,
        // Default level is "Trace"
        log_level: None,
        // Default is 15 pings every 60 seconds
        rate_limit: None,
        // Wheter to add a wallclock timestamp to all events
        enable_event_timestamps: false,
        experimentation_id: None,
        enable_internal_pings: true,
    };

    let client_info = ClientInfoMetrics {
        app_build: env!("BUILD_ID").to_string(),
        app_display_version: env!("APP_VERSION").to_string(),
        channel: channel.to_string_fallible().ok(),
        locale: locale.to_string_fallible().ok(),
    };

    register_pings();
    glean::initialize(cfg, client_info);
}

#[no_mangle]
pub extern "C" fn glean_set_upload_enabled(is_telemetry_enabled: bool) {
    glean::set_upload_enabled(is_telemetry_enabled);
}

#[no_mangle]
pub extern "C" fn glean_set_debug_view_tag(tag: FfiStr) {
    glean::set_debug_view_tag(tag.as_str());
}

#[no_mangle]
pub extern "C" fn glean_set_log_pings(flag: bool) {
    glean::set_log_pings(flag);
}

#[no_mangle]
pub extern "C" fn glean_shutdown() {
    glean::shutdown();
}

#[no_mangle]
pub extern "C" fn glean_test_reset_glean(is_telemetry_enabled: bool, data_path: FfiStr, locale: FfiStr) {
    let cfg = Configuration {
        data_path: data_path
            .to_string_fallible()
            .expect("Invalid Glean data path.")
            .into(),
        application_id: GLEAN_APPLICATION_ID.into(),
        upload_enabled: is_telemetry_enabled,
        // Default is 500
        max_events: None,
        // Whether Glean should delay persistence of data from metrics with ping lifetime
        delay_ping_lifetime_io: false,
        // Default is "https://incoming.telemetry.mozilla.org"
        server_endpoint: None,
        // Once https://bugzilla.mozilla.org/show_bug.cgi?id=1675468 is resolved
        // we will need to actually create a custom no-op one.
        // We want a no-op uploader for tests.
        uploader: None,
        // Whether Glean should schedule “metrics” pings for you
        use_core_mps: true,
        trim_data_to_registered_pings: false,
        // Default level is "Trace"
        log_level: None,
        // Default is 15 pings every 60 seconds
        rate_limit: None,
        // Wheter to add a wallclock timestamp to all events
        enable_event_timestamps: false,
        experimentation_id: None,
        enable_internal_pings: true,
    };

    let client_info = ClientInfoMetrics {
        app_build: env!("BUILD_ID").to_string(),
        app_display_version: env!("APP_VERSION").to_string(),
        channel: Some("testing".to_string()),
        locale: locale.to_string_fallible().ok(),
    };

    glean::test_reset_glean(cfg, client_info, true);
}

#[no_mangle]
pub extern "C" fn glean_push_ping_filter(ping_name: FfiStr) {
    let mut list = GLOBAL_PING_FILTER_LIST.lock().unwrap();
    list.push(ping_name.into_string());
}
#[no_mangle]
pub extern "C" fn glean_clear_ping_filter() {
    let mut list = GLOBAL_PING_FILTER_LIST.lock().unwrap();
    list.clear();
}
