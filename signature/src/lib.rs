/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use crate::balrog::*;
use crate::logger::*;
use ring::digest;
use ring::signature;
use std::ffi::CStr;
use std::os::raw::c_char;
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
    log_fn: Option<extern "C" fn(*const c_char)>,
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

/* FFI interface to get the SHA256 hash of the root certificate in a chain
 *
 * The following arguments are expected:
 *  - x5u_ptr: Pointer to a PEM-encoded X509 certificate chain.
 *  - x5u_length: Length of PEM-encoded X509 certificate chain (in bytes).
 *  - root_hash_out: Output pointer for the SHA256 hash.
 *  - root_hash_len: Size of the root hash buffer (in bytes).
 */
#[no_mangle]
pub extern "C" fn compute_root_certificate_hash(
    x5u_ptr: *const c_uchar,
    x5u_length: usize,
    root_hash_out: *mut c_uchar,
    root_hash_len: usize,
) -> bool {
    /* Parse the PEM Certificate chain. */
    let x5u = unsafe { std::slice::from_raw_parts(x5u_ptr, x5u_length) };
    let Ok(pem_chain) = parse_pem_chain(x5u) else {
        return false;
    };

    /* Hash the final certificate in the chain. */
    let Some(pem) = pem_chain.last() else {
        return false;
    };
    if pem.label != "CERTIFICATE" {
        return false;
    }
    let digest = digest::digest(&digest::SHA256, pem.contents.as_slice());

    /* Output the hash to the caller. */
    if root_hash_len != digest::SHA256_OUTPUT_LEN {
        return false;
    }
    let output = unsafe { std::slice::from_raw_parts_mut(root_hash_out, root_hash_len) };
    output.copy_from_slice(digest.as_ref());
    return true;
}

/* FFI interface to verify a Mozilla content signature
 *
 * The following arguments are expected:
 *  - x5u_ptr: Pointer to a PEM-encoded X509 certificate chain.
 *  - x5u_length: Length of PEM-encoded X509 certificate chain (in bytes).
 *  - input_ptr: Data signed by the leaf certificate.
 *  - input_length: Length of data signed by the leaf certificate (in bytes).
 *  - signature: base64-encoded content signature.
 *  - leaf_subject: hostname from which the content signature was received.
 *  - log_fn: callback function for log messages (optional)
 */
#[no_mangle]
pub extern "C" fn verify_content_signature(
    x5u_ptr: *const c_uchar,
    x5u_length: usize,
    input_ptr: *const c_uchar,
    input_length: usize,
    signature: *const c_char,
    leaf_subject: *const c_char,
    log_fn: Option<extern "C" fn(*const c_char)>,
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
    let leaf_subject_str = match unsafe { CStr::from_ptr(leaf_subject) }.to_str() {
        Err(e) => {
            logger.print(&e.to_string());
            return false;
        }
        Ok(x) => x,
    };

    #[cfg(not(test))]
    let now: i64 = ASN1Time::now().timestamp();
    #[cfg(test)]
    let now = test::mock_x5u_timestamp(x5u);

    let pem_chain = match parse_pem_chain(x5u) {
        Err(e) => {
            logger.print(&e.to_string());
            return false;
        }
        Ok(x) => x,
    };
    let balrog = match Balrog::new(&pem_chain) {
        Err(e) => {
            logger.print(&e.to_string());
            return false;
        }
        Ok(x) => x,
    };

    /* Perform the content signature validation. */
    let _ = match balrog.verify(&input, &sig_str, now, leaf_subject_str) {
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

    /* Extract a timestamp from a certificate, suitable for mocking. */
    #[cfg(test)]
    pub fn mock_x5u_timestamp(x5u: &[u8]) -> i64 {
        let Ok((_rem, pem)) = parse_x509_pem(x5u) else {
            return 0;
        };
        let Ok((_rem, x509)) = parse_x509_certificate(&pem.contents) else {
            return 0;
        };

        let v = x509.validity();
        (v.not_before.timestamp() + v.not_after.timestamp()) / 2
    }

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
    fn test_rsa_ffi_logger() {
        let mut addon_message = data_encoding::BASE64
            .decode(RSA_ADDON_MESSAGE_BASE64)
            .unwrap();
        addon_message.extend_from_slice(b"Hello World");

        // Capture the FFI log message via a callback.
        static mut LOG_MESSAGE: String = String::new();
        extern "C" fn callback(msg: *const c_char) {
            unsafe { LOG_MESSAGE.push_str(CStr::from_ptr(msg).to_str().unwrap()); }
        }

        let r = verify_rsa(
            RSA_PUBLIC_KEY.as_ptr(),
            RSA_PUBLIC_KEY.len(),
            addon_message.as_ptr(),
            addon_message.len(),
            RSA_ADDON_SIGNATURE.as_ptr(),
            RSA_ADDON_SIGNATURE.len(),
            Some(callback),
        );
        assert!(!r, "RSA signature check failed to catch an error");
        unsafe { assert_eq!(LOG_MESSAGE, "ring::error::Unspecified"); }
    }

    #[test]
    fn test_compute_root_hash() {
        let mut hash_result = [0u8; digest::SHA256_OUTPUT_LEN];
        let expect = hex::decode(PROD_ROOT_HASH).unwrap();

        let r = compute_root_certificate_hash(
            PROD_CERT_CHAIN.as_ptr(),
            PROD_CERT_CHAIN.len(),
            hash_result.as_mut_ptr(),
            hash_result.len(),
        );

        assert!(r, "Root hash computation failed");
        assert_eq!(hash_result, expect.as_slice());
    }

    #[test]
    fn test_compute_root_hash_reject_bad_pem() {
        let mut hash_result = [0u8; digest::SHA256_OUTPUT_LEN];
        let pem_empty: &[u8] = b"";
        let pem_bad_contents: &[u8] = b"\
-----BEGIN CERTIFICATE-----
Trust me, I am a certificate.
-----END CERTIFICATE-----";
        let pem_not_cert: &[u8] = b"\
-----BEGIN COMPLIMENT-----
WW91IGFyZSBhd2Vzb21lISBHb29kIEpvYiEK
-----END COMPLIMENT-----";

        let r = compute_root_certificate_hash(
            pem_empty.as_ptr(),
            pem_empty.len(),
            hash_result.as_mut_ptr(),
            hash_result.len(),
        );
        assert!(!r, "Root hash computation failed to detect empty PEM content");

        let r = compute_root_certificate_hash(
            pem_bad_contents.as_ptr(),
            pem_bad_contents.len(),
            hash_result.as_mut_ptr(),
            hash_result.len(),
        );
        assert!(!r, "Root hash computation failed to detect invalid PEM content");

        let r = compute_root_certificate_hash(
            pem_not_cert.as_ptr(),
            pem_not_cert.len(),
            hash_result.as_mut_ptr(),
            hash_result.len(),
        );
        assert!(!r, "Root hash computation failed to detect invalid PEM type");
    }

    #[test]
    fn test_compute_root_hash_invalid_length() {
        let mut hash_result = [0u8; digest::SHA256_OUTPUT_LEN - 1];

        let r = compute_root_certificate_hash(
            PROD_CERT_CHAIN.as_ptr(),
            PROD_CERT_CHAIN.len(),
            hash_result.as_mut_ptr(),
            hash_result.len(),
        );

        assert!(!r, "Root hash computation failed to check buffer size");
    }

    #[test]
    fn test_verify_prod_example() {
        let chain = parse_pem_chain(PROD_CERT_CHAIN).unwrap();
        let balrog = Balrog::new(&chain).unwrap();

        let r = balrog.verify(
            PROD_INPUT_DATA,
            PROD_SIGNATURE,
            mock_x5u_timestamp(PROD_CERT_CHAIN),
            PROD_HOSTNAME,
        );
        assert!(r.is_ok(), "Found unexpected error: {}", r.unwrap_err());
    }

    #[test]
    fn test_verify_valid_ffi() {
        let prod_signature_cstr = CString::new(PROD_SIGNATURE).unwrap().into_raw();
        let prod_hostname_cstr = CString::new(PROD_HOSTNAME).unwrap().into_raw();

        let r = verify_content_signature(
            PROD_CERT_CHAIN.as_ptr(),
            PROD_CERT_CHAIN.len(),
            PROD_INPUT_DATA.as_ptr(),
            PROD_INPUT_DATA.len(),
            prod_signature_cstr,
            prod_hostname_cstr,
            None,
        );
        assert!(r, "Verification failed via FFI");

        // Retake pointers for garbage collection.
        unsafe {
            let _ = CString::from_raw(prod_signature_cstr);
            let _ = CString::from_raw(prod_hostname_cstr);
        };
    }

    #[test]
    fn test_verify_ffi_logger() {
        let prod_signature_cstr = CString::new(PROD_SIGNATURE).unwrap().into_raw();
        let invalid_hostname_cstr = CString::new("example.com").unwrap().into_raw();

        // Capture the FFI log message via a callback.
        static mut LOG_MESSAGE: String = String::new();
        extern "C" fn callback(msg: *const c_char) {
            unsafe { LOG_MESSAGE.push_str(CStr::from_ptr(msg).to_str().unwrap()); }
        }

        let r = verify_content_signature(
            PROD_CERT_CHAIN.as_ptr(),
            PROD_CERT_CHAIN.len(),
            PROD_INPUT_DATA.as_ptr(),
            PROD_INPUT_DATA.len(),
            prod_signature_cstr,
            invalid_hostname_cstr,
            Some(callback),
        );
        assert!(!r, "Verification failed to catch invalid hostname via FFI");
        unsafe { assert_eq!(LOG_MESSAGE, "Hostname mismatch"); };

        // Retake pointers for garbage collection.
        unsafe {
            let _ = CString::from_raw(prod_signature_cstr);
            let _ = CString::from_raw(invalid_hostname_cstr);
        };
    }
}
