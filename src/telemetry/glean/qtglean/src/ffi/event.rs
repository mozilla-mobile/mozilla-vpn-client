// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use std::ffi::CString;
use std::os::raw::c_char;

use ffi_support::FfiStr;
use glean::traits::EventRecordingError;
use glean::ErrorType;

use crate::ffi::helpers;
use crate::ffi::helpers::RawStringArray;
use crate::metrics::__generated_metrics as metric_maps;

#[no_mangle]
pub extern "C" fn glean_event_record_no_extra(id: u32) {
    match metric_maps::record_event_by_id_no_extra(id) {
        Ok(()) => {}
        Err(EventRecordingError::InvalidId) => panic!("No event for id {}", id),
        Err(EventRecordingError::InvalidExtraKey) => {
            // TODO: Record an error.
        }
    }
}

#[no_mangle]
pub extern "C" fn glean_event_record(
    id: u32,
    extra_keys: RawStringArray,
    extra_values: RawStringArray,
    extras_len: i32,
) {
    let extras = helpers::from_raw_string_array(extra_keys, extra_values, extras_len).unwrap();
    match metric_maps::record_event_by_id(id, extras.unwrap()) {
        Ok(()) => {}
        Err(EventRecordingError::InvalidId) => panic!("No event for id {}", id),
        Err(EventRecordingError::InvalidExtraKey) => {
            // TODO: Record an error.
        }
    }
}

#[no_mangle]
pub extern "C" fn glean_event_test_get_num_recorded_errors(id: u32, error_type: ErrorType) -> i32 {
    metric_maps::event_test_get_num_recorded_errors(
        id,
        glean::ErrorType::try_from(error_type).expect("Invalid error type."),
    )
}

#[no_mangle]
pub extern "C" fn glean_event_test_get_value(id: u32, ping_name: FfiStr) -> *mut c_char {
    let value_as_json = if let Some(value) = metric_maps::event_test_get_value(id, helpers::option_from_ffi(ping_name)) {
        serde_json::to_string(&value).expect("Unable to serialize recorded value")
    } else {
        "".into()
    };

    CString::new(value_as_json)
        .expect("Unable to create CString.")
        .into_raw()
}
