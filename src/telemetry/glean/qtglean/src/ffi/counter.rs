// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use ffi_support::FfiStr;
use glean::ErrorType;

use crate::ffi::helpers;

#[no_mangle]
pub extern "C" fn glean_counter_add(id: u32, amount: i32) {
    with_metric!(COUNTER_MAP, id, metric, metric.add(amount))
}

#[no_mangle]
pub extern "C" fn glean_counter_test_get_value(
    id: u32,
    ping_name: FfiStr
) -> i32 {
    with_metric!(
        COUNTER_MAP,
        id,
        metric,
        metric.test_get_value(helpers::option_from_ffi(ping_name)).unwrap_or(0)
    )
}

#[no_mangle]
pub extern "C" fn glean_counter_test_get_num_recorded_errors(
    id: u32,
    error_type: ErrorType,
) -> i32 {
    with_metric!(
        COUNTER_MAP,
        id,
        metric,
        metric.test_get_num_recorded_errors(error_type)
    )
}
