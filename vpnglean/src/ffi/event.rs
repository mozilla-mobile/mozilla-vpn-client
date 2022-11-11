// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use glean::traits::EventRecordingError;

use crate::ffi::raw_string_array::{from_raw_string_array, RawStringArray};
use crate::metrics::__generated_metrics as metric_maps;

#[no_mangle]
pub extern "C" fn glean_event_record_no_extra(id: u32) {
    match metric_maps::record_event_by_id_no_extra(id) {
        Ok(()) => {}
        Err(EventRecordingError::InvalidId) => panic!("No event for id {}", id),
        Err(EventRecordingError::InvalidExtraKey) => {
            // TODO: Record an error. bug 1704504.
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
    let extras = from_raw_string_array(extra_keys, extra_values, extras_len).unwrap();
    match metric_maps::record_event_by_id(id, extras.unwrap()) {
        Ok(()) => {}
        Err(EventRecordingError::InvalidId) => panic!("No event for id {}", id),
        Err(EventRecordingError::InvalidExtraKey) => {
            // TODO: Record an error. bug 1704504.
        }
    }
}
