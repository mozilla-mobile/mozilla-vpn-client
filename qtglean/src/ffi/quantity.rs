// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use ffi_support::FfiStr;
use glean::ErrorType;

use crate::ffi::helpers;

#[no_mangle]
pub extern "C" fn glean_quantity_set(id: u32, value: i64) {
    with_metric!(QUANTITY_MAP, id, metric, metric.set(value))
}

#[no_mangle]
pub extern "C" fn glean_quantity_test_get_value(
    id: u32,
    ping_name: FfiStr
) -> i64 {
    with_metric!(
        QUANTITY_MAP,
        id,
        metric,
        metric.test_get_value(helpers::ping_name_from_ffi(ping_name)).unwrap_or(0)
    )
}

#[no_mangle]
pub extern "C" fn glean_quantity_test_get_num_recorded_errors(
    id: u32,
    error_type: ErrorType,
) -> i32 {
    with_metric!(
        QUANTITY_MAP,
        id,
        metric,
        metric.test_get_num_recorded_errors(error_type)
    )
}
