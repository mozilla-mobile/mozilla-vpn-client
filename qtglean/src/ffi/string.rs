// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use ffi_support::FfiStr;
use std::ffi::CString;
use std::os::raw::c_char;
use glean::ErrorType;

use crate::ffi::helpers;

#[no_mangle]
pub extern "C" fn glean_string_set(id: u32, value: FfiStr) {
    with_metric!(STRING_MAP, id, metric, metric.set(value.into_string()))
}

#[no_mangle]
pub extern "C" fn glean_string_test_get_value(
    id: u32,
    ping_name: FfiStr
) -> *mut c_char {
    let returnString = with_metric!(
        STRING_MAP,
        id,
        metric,
        metric.test_get_value(helpers::ping_name_from_ffi(ping_name)).unwrap_or("".to_string())
    );

    CString::new(returnString)
        .expect("Unable to create CString.")
        .into_raw()
}

#[no_mangle]
pub extern "C" fn glean_string_test_get_num_recorded_errors(
    id: u32,
    error_type: ErrorType,
) -> i32 {
    with_metric!(
        STRING_MAP,
        id,
        metric,
        metric.test_get_num_recorded_errors(error_type)
    )
}
