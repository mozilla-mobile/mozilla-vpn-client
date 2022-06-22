/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use ring::signature;
use std::os::raw::c_uchar;

#[no_mangle]
pub extern "C" fn verify_rsa(
    public_key_ptr: *const c_uchar,
    public_key_length: usize,
    message_ptr: *const c_uchar,
    message_length: usize,
    message_signature_ptr: *const c_uchar,
    message_signature_length: usize,
) -> bool {
    let public_key_str = unsafe { std::slice::from_raw_parts(public_key_ptr, public_key_length) };
    let public_key =
        signature::UnparsedPublicKey::new(&signature::RSA_PKCS1_2048_8192_SHA256, &public_key_str);

    let message_str = unsafe { std::slice::from_raw_parts(message_ptr, message_length) };
    let message_signature_str =
        unsafe { std::slice::from_raw_parts(message_signature_ptr, message_signature_length) };

    match public_key.verify(message_str, message_signature_str) {
        Err(e) => {
            eprintln!("{}", e);
            false
        }
        Ok(_) => true,
    }
}
