/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use ffi_support::FfiStr;

use std::collections::HashMap;
use std::os::raw::c_char;

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

pub fn ping_name_from_ffi(ping_name: FfiStr) -> Option<String> {
    if let Ok(name) = ping_name.to_string_fallible() {
        if name.is_empty() {
            None
        } else {
            Some(name)
        }
    } else {
        None
    }
}
