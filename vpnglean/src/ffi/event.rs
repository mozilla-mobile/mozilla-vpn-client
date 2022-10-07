// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use std::collections::HashMap;
use std::os::raw::c_char;

use crate::metrics::__generated_metrics as metric_maps;
use ffi_support::FfiStr;
use glean::traits::EventRecordingError;

// Copied from glean-core:
// https://github.com/mozilla/glean/blob/b7176d4d89b6497364ed0fc86cd0c1e7b07269b7/glean-core/ffi/src/ffi_string_ext.rs

pub trait FallibleToString {
    /// Convert to a string or fail with an appropriate error.
    fn to_string_fallible(&self) -> glean::Result<String>;
}

/// This allows to convert a `FfiStr` (effectively a null-terminated C-like string)
/// to a Rust string, failing when the pointer is null or contains invalid UTF-8 characters.
impl<'a> FallibleToString for FfiStr<'a> {
    fn to_string_fallible(&self) -> glean::Result<String> {
        self.as_opt_str()
            .map(|s| s.to_string())
            .ok_or_else(glean::Error::utf8_error)
    }
}

// Copied from glean-core:
// https://github.com/mozilla/glean/blob/b7176d4d89b6497364ed0fc86cd0c1e7b07269b7/glean-core/ffi/src/from_raw.rs

pub type RawStringArray = *const *const c_char;

/// Creates a HashMap<String, String> from a pair of C string arrays.
///
/// Returns an error if any of the strings contain invalid UTF-8 characters.
///
/// # Safety
///
/// * We check the array pointer for validity (non-null).
/// * FfiStr checks each individual char pointer for validity (non-null).
/// * We discard invalid char pointers (null pointer).
/// * Invalid UTF-8 in any string will return an error from this function.
pub fn from_raw_string_array(
    keys: RawStringArray,
    values: RawStringArray,
    len: i32,
) -> glean::Result<Option<HashMap<String, String>>> {
    unsafe {
        if keys.is_null() || values.is_null() || len <= 0 {
            return Ok(None);
        }

        let keys_ptrs = std::slice::from_raw_parts(keys, len as usize);
        let values_ptrs = std::slice::from_raw_parts(values, len as usize);

        let res: glean::Result<_> = keys_ptrs
            .iter()
            .zip(values_ptrs.iter())
            .map(|(&k, &v)| {
                let k = FfiStr::from_raw(k).to_string_fallible()?;

                let v = FfiStr::from_raw(v).to_string_fallible()?;

                Ok((k, v))
            })
            .collect();
        res.map(Some)
    }
}

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

#[no_mangle]
pub extern "C" fn glean_event_test_get_error(id: u32) -> bool {
    let err = metric_maps::event_test_get_error(id);
    err.is_some()
}
