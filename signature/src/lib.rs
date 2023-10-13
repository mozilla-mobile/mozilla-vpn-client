/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use crate::balrog::*;
use crate::logger::*;
use ring::signature;
use std::ffi::CStr;
use std::os::raw::c_uchar;
use x509_parser::prelude::*;

pub mod balrog;
pub mod logger;

/* FFI interface to verify an RSA signature
 *
 * The following arguments are expected:
 *  - public_key_ptr: Pointer to a DER-encoded RSA public key.
 *  - public_key_length: Length of DER-encoded RSA public key (in bytes).
 *  - message_ptr: Data signed by the RSA public key.
 *  - message_length: Length of data signed by the RSA public key (in bytes).
 *  - message_signature_ptr: Pointer to the RSA signature to verify.
 *  - message_signature_length: Length of the RSA signature (in bytes).
 *  - log_fn: callback function for log messages (optional)
 */
#[no_mangle]
pub extern "C" fn verify_rsa(
    public_key_ptr: *const c_uchar,
    public_key_length: usize,
    message_ptr: *const c_uchar,
    message_length: usize,
    message_signature_ptr: *const c_uchar,
    message_signature_length: usize,
    log_fn: Option<extern "C" fn(*const i8)>,
) -> bool {
    let logger = SignatureLogger { handler: log_fn };

    let public_key_str = unsafe { std::slice::from_raw_parts(public_key_ptr, public_key_length) };
    let public_key =
        signature::UnparsedPublicKey::new(&signature::RSA_PKCS1_2048_8192_SHA256, &public_key_str);

    let message_str = unsafe { std::slice::from_raw_parts(message_ptr, message_length) };
    let message_signature_str =
        unsafe { std::slice::from_raw_parts(message_signature_ptr, message_signature_length) };

    match public_key.verify(message_str, message_signature_str) {
        Err(e) => {
            logger.print(&e.to_string());
            false
        }
        Ok(_) => true,
    }
}

/* FFI interface to verify a Mozilla content signature
 *
 * The following arguments are expected:
 *  - x5u_ptr: Pointer to a PEM-encoded X509 certificate chain.
 *  - x5u_length: Length of PEM-encoded X509 certificate chain (in bytes).
 *  - input_ptr: Data signed by the leaf certificate.
 *  - input_length: Length of data signed by the leaf certificate (in bytes).
 *  - signature: base64-encoded content signature.
 *  - root_hash: hex-encoded SHA256 hash expected of the root certificate.
 *  - leaf_subject: hostname from which the content signature was received.
 *  - log_fn: callback function for log messages (optional)
 */
#[no_mangle]
pub extern "C" fn verify_content_signature(
    x5u_ptr: *const c_uchar,
    x5u_length: usize,
    input_ptr: *const c_uchar,
    input_length: usize,
    signature: *const i8,
    root_hash: *const i8,
    leaf_subject: *const i8,
    log_fn: Option<extern "C" fn(*const i8)>,
) -> bool {
    let logger = SignatureLogger { handler: log_fn };

    /* Translate C/FFI arguments into Rust types. */
    let x5u = unsafe { std::slice::from_raw_parts(x5u_ptr, x5u_length) };
    let input = unsafe { std::slice::from_raw_parts(input_ptr, input_length) };
    let sig_str = match unsafe { CStr::from_ptr(signature) }.to_str() {
        Err(e) => {
            logger.print(&e.to_string());
            return false;
        }
        Ok(x) => x,
    };
    let root_hash_str = match unsafe { CStr::from_ptr(root_hash) }.to_str() {
        Err(e) => {
            logger.print(&e.to_string());
            return false;
        }
        Ok(x) => x,
    };
    let leaf_subject_str = match unsafe { CStr::from_ptr(leaf_subject) }.to_str() {
        Err(e) => {
            logger.print(&e.to_string());
            return false;
        }
        Ok(x) => x,
    };
    let now: i64 = ASN1Time::now().timestamp();

    /* Perform the content signature validation. */
    let _ = match parse_and_verify(&x5u, &input, &sig_str, now, root_hash_str, leaf_subject_str) {
        Err(e) => {
            logger.print(&e.to_string());
            return false;
        }
        Ok(x) => x,
    };

    /* Success! */
    true
}

#[cfg(test)]
mod test {
    use super::*;
    use std::ffi::CString;

    // An RSA signature of an addon manifest from prod.
    const RSA_PUBLIC_KEY: &[u8] = include_bytes!("../../src/resources/public_keys/production.der");
    const RSA_ADDON_MESSAGE_BASE64: &[u8] = include_bytes!("../assets/addon_manifest_base64.txt");
    const RSA_ADDON_SIGNATURE: &[u8] = include_bytes!("../assets/addon_signature.sig");

    // Fetched from: https://content-signature-2.cdn.mozilla.net/chains/aus.content-signature.mozilla.org-2023-11-18-16-07-40.chain
    const PROD_CERT_CHAIN: &[u8] =
        include_bytes!("../assets/aus.content-signature.mozilla.org-2023-11-18-16-07-40.chain");
    const PROD_ROOT_HASH: &str = "97e8ba9cf12fb3de53cc42a4e6577ed64df493c247b414fea036818d3823560e";
    const PROD_HOSTNAME: &str = "aus.content-signature.mozilla.org";
    // Fetched from: https://aus5.mozilla.org/json/1/FirefoxVPN/2.14.0/WINNT_x86_64/release-cdntest/update.json
    const PROD_SIGNATURE: &str = "znYFqdKKFgijVgUhnq5VuZxtI5Zay8MARVFr3cG1CbB9eH9slQFkE9ZjMdLzbf5OZqj2gds1OqbCm45L38e2joKD_mCAUGtajebztDdWx9Rqgmn-9vu6t-SCl6HQrzbh";
    const PROD_INPUT_DATA: &[u8] = include_bytes!("../assets/prod_update_data.json");

    #[test]
    fn test_rsa_success() {
        let addon_message = data_encoding::BASE64
            .decode(RSA_ADDON_MESSAGE_BASE64)
            .unwrap();

        let r = verify_rsa(
            RSA_PUBLIC_KEY.as_ptr(),
            RSA_PUBLIC_KEY.len(),
            addon_message.as_ptr(),
            addon_message.len(),
            RSA_ADDON_SIGNATURE.as_ptr(),
            RSA_ADDON_SIGNATURE.len(),
            None,
        );
        assert!(r, "RSA signature check failed");
    }

    #[test]
    fn test_rsa_fails_if_addded_padding() {
        /* Extending the message by adding padding. */
        let mut addon_message = data_encoding::BASE64
            .decode(RSA_ADDON_MESSAGE_BASE64)
            .unwrap();
        addon_message.extend_from_slice(b"Hello World");

        let r = verify_rsa(
            RSA_PUBLIC_KEY.as_ptr(),
            RSA_PUBLIC_KEY.len(),
            addon_message.as_ptr(),
            addon_message.len(),
            RSA_ADDON_SIGNATURE.as_ptr(),
            RSA_ADDON_SIGNATURE.len(),
            None,
        );
        assert!(!r, "RSA signature check failed to catch added padding");
    }

    #[test]
    fn test_rsa_fails_if_bad_signature() {
        /* Flip a bit in the middle of the signature. */
        let mut bad_signature = RSA_ADDON_SIGNATURE.to_vec();
        bad_signature[RSA_ADDON_SIGNATURE.len() / 2] ^= 0x80;

        let addon_message = data_encoding::BASE64
            .decode(RSA_ADDON_MESSAGE_BASE64)
            .unwrap();
        let r = verify_rsa(
            RSA_PUBLIC_KEY.as_ptr(),
            RSA_PUBLIC_KEY.len(),
            addon_message.as_ptr(),
            addon_message.len(),
            bad_signature.as_ptr(),
            bad_signature.len(),
            None,
        );
        assert!(!r, "RSA signature check failed to catch a bad signature");
    }

    #[test]
    #[should_panic]
    fn test_rsa_ffi_logger() {
        extern "C" fn logfn(msg: *const i8) {
            match unsafe { CStr::from_ptr(msg) }.to_str() {
                Err(_e) => {}
                Ok(x) => panic!("{}", x),
            };
        }

        let mut addon_message = data_encoding::BASE64
            .decode(RSA_ADDON_MESSAGE_BASE64)
            .unwrap();
        addon_message.extend_from_slice(b"Hello World");

        let r = verify_rsa(
            RSA_PUBLIC_KEY.as_ptr(),
            RSA_PUBLIC_KEY.len(),
            addon_message.as_ptr(),
            addon_message.len(),
            RSA_ADDON_SIGNATURE.as_ptr(),
            RSA_ADDON_SIGNATURE.len(),
            Some(logfn),
        );
        assert!(!r, "RSA signature check failed to catch an error");
    }

    #[test]
    fn test_verify_prod_example() {
        let r = parse_and_verify(
            PROD_CERT_CHAIN,
            PROD_INPUT_DATA,
            PROD_SIGNATURE,
            1696272799, // Oct 2, 2023 - the time when I wrote this test
            PROD_ROOT_HASH,
            PROD_HOSTNAME,
        );
        assert!(r.is_ok(), "Found unexpected error: {}", r.unwrap_err());
    }

    #[test]
    fn test_verify_prod_ffi() {
        let prod_signature_cstr = CString::new(PROD_SIGNATURE).unwrap().into_raw();
        let prod_root_hash_cstr = CString::new(PROD_ROOT_HASH).unwrap().into_raw();
        let prod_hostname_cstr = CString::new(PROD_HOSTNAME).unwrap().into_raw();

        // TODO: Is there a way to mock the timestamp?
        // Otherwise this test will mysteriously start to fail sometime
        // in the future when the certificates start to expire.
        let r = verify_content_signature(
            PROD_CERT_CHAIN.as_ptr(),
            PROD_CERT_CHAIN.len(),
            PROD_INPUT_DATA.as_ptr(),
            PROD_INPUT_DATA.len(),
            prod_signature_cstr,
            prod_root_hash_cstr,
            prod_hostname_cstr,
            None,
        );
        assert!(r, "Verification failed via FFI");

        // Retake pointers for garbage collection.
        unsafe {
            let _ = CString::from_raw(prod_signature_cstr);
            let _ = CString::from_raw(prod_root_hash_cstr);
            let _ = CString::from_raw(prod_hostname_cstr);
        };
    }

    #[test]
    #[should_panic(expected = "Hostname mismatch")]
    fn test_verify_ffi_logger() {
        let prod_signature_cstr = CString::new(PROD_SIGNATURE).unwrap().into_raw();
        let prod_root_hash_cstr = CString::new(PROD_ROOT_HASH).unwrap().into_raw();
        let invalid_hostname_cstr = CString::new("example.com").unwrap().into_raw();

        extern "C" fn logfn(msg: *const i8) {
            match unsafe { CStr::from_ptr(msg) }.to_str() {
                Err(_e) => {}
                Ok(x) => panic!("{}", x),
            };
        }

        let r = verify_content_signature(
            PROD_CERT_CHAIN.as_ptr(),
            PROD_CERT_CHAIN.len(),
            PROD_INPUT_DATA.as_ptr(),
            PROD_INPUT_DATA.len(),
            prod_signature_cstr,
            prod_root_hash_cstr,
            invalid_hostname_cstr,
            Some(logfn),
        );
        assert!(!r, "Verification failed to catch invalid hostname via FFI");

        // Retake pointers for garbage collection.
        unsafe {
            let _ = CString::from_raw(prod_signature_cstr);
            let _ = CString::from_raw(prod_root_hash_cstr);
            let _ = CString::from_raw(invalid_hostname_cstr);
        };
    }
}
