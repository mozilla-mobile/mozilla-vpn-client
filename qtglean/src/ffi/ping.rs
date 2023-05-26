// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use crate::metrics::__generated_pings::submit_ping_by_id;

#[no_mangle]
pub extern "C" fn glean_submit_ping_by_id(id: u32, value: FfiStr) {
    let reason = reason.into_string();
    let reason = reason.is_empty() {
        None
    } else {
        Some(reason)
    };

    submit_ping_by_id(id, reason);
}
