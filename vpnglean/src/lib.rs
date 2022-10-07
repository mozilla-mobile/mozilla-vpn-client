#[macro_use]
pub mod macros;

use std::env;

// Make internal Glean symbols public for mobile SDK consumption.
pub use glean_core;

use glean::{ClientInfoMetrics, Configuration};
use uploader::VPNPingUploader;

pub mod ffi;

pub mod metrics;
mod uploader;

#[no_mangle]
pub extern "C" fn glean_initialize(is_telemetry_enabled: bool) {
    let cfg = Configuration {
        data_path: env::temp_dir(),
        application_id: "my-app-id".into(),
        upload_enabled: is_telemetry_enabled,
        max_events: None,
        delay_ping_lifetime_io: false,
        server_endpoint: Some("https://incoming.telemetry.mozilla.org".into()),
        uploader: Some(Box::new(VPNPingUploader::new())),
        use_core_mps: true,
    };

    let client_info = ClientInfoMetrics {
        app_build: env!("CARGO_PKG_VERSION").to_string(),
        app_display_version: env!("CARGO_PKG_VERSION").to_string(),
        channel: None,
    };

    glean::initialize(cfg, client_info);
    // glean::set_debug_view_tag("glean-rs-vpn");
    glean::set_log_pings(true);
}
