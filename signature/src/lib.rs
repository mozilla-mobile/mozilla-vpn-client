/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use crate::balrog::*;
use ring::signature;
use std::ffi::CStr;
use std::os::raw::c_uchar;
use x509_parser::prelude::*;

pub mod balrog;

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

/* FFI interface to verify a Mozilla content signature
 *
 * The following arguments are expected:
 *  - x5u_ptr: Pointer to a PEM-encoded X509 certificate chain.
 *  - x5u_length: Length of PEM-encoded X509 certificate chain.
 *  - input_ptr: Data signed by the leaf certificate.
 *  - input_length: Length of data signed by the leaf certificate.
 *  - signature: base64-encoded content signature.
 *  - root_hash: hex-encoded SHA256 hash expected of the root certificate.
 *  - leaf_subject: hostname from which the content signature was received.
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
) -> bool {
    /* Translate C/FFI arguments into Rust types. */
    let x5u = unsafe { std::slice::from_raw_parts(x5u_ptr, x5u_length) };
    let input = unsafe { std::slice::from_raw_parts(input_ptr, input_length) };
    let sig_str = match unsafe { CStr::from_ptr(signature) }.to_str() {
        Err(e) => {
            eprintln!("{}", e);
            return false;
        }
        Ok(x) => x,
    };
    let root_hash_str = match unsafe { CStr::from_ptr(root_hash) }.to_str() {
        Err(e) => {
            eprintln!("{}", e);
            return false;
        }
        Ok(x) => x,
    };
    let leaf_subject_str = match unsafe { CStr::from_ptr(leaf_subject) }.to_str() {
        Err(e) => {
            eprintln!("{}", e);
            return false;
        }
        Ok(x) => x,
    };
    let now: i64 = ASN1Time::now().timestamp();

    /* Perform the content signature validation. */
    let _ = match parse_and_verify(&x5u, &input, &sig_str, now, root_hash_str, leaf_subject_str) {
        Err(e) => {
            eprintln!("{}", e);
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

    // Copied from https://github.com/mozilla/application-services/blob/main/components/support/rc_crypto/src/contentsignature.rs
    const ROOT_HASH: &str = "3C:01:44:6A:BE:90:36:CE:A9:A0:9A:CA:A3:A5:20:AC:62:8F:20:A7:AE:32:CE:86:1C:B2:EF:B7:0F:A0:C7:45";
    const VALID_CERT_CHAIN: &[u8] = include_bytes!("../assets/valid_cert_chain.pem");
    const VALID_INPUT: &[u8] = b"{\"data\":[],\"last_modified\":\"1603992731957\"}";
    const VALID_SIGNATURE: &str = "fJJcOpwdnkjEWFeHXfdOJN6GaGLuDTPGzQOxA2jn6ldIleIk6KqMhZcy2GZv2uYiGwl6DERWwpaoUfQFLyCAOcVjck1qlaaEFZGY1BQba9p99xEc9FNQ3YPPfvSSZqsw";
    const VALID_HOSTNAME: &str = "remote-settings.content-signature.mozilla.org";
    const VALID_TIMESTAMP: i64 = 1615559719; // March 12, 2021

    const INVALID_CERTIFICATE_DER: &[u8] = include_bytes!("../assets/invalid_der_content.pem");

    // Fetched from: https://content-signature-2.cdn.mozilla.net/chains/aus.content-signature.mozilla.org-2023-11-18-16-07-40.chain
    const PROD_CERT_CHAIN: &[u8] =
        include_bytes!("../assets/aus.content-signature.mozilla.org-2023-11-18-16-07-40.chain");
    const PROD_ROOT_HASH: &str = "97e8ba9cf12fb3de53cc42a4e6577ed64df493c247b414fea036818d3823560e";
    const PROD_HOSTNAME: &str = "aus.content-signature.mozilla.org";
    // Fetched from: https://aus5.mozilla.org/json/1/FirefoxVPN/2.14.0/WINNT_x86_64/release-cdntest/update.json
    const PROD_SIGNATURE: &str = "znYFqdKKFgijVgUhnq5VuZxtI5Zay8MARVFr3cG1CbB9eH9slQFkE9ZjMdLzbf5OZqj2gds1OqbCm45L38e2joKD_mCAUGtajebztDdWx9Rqgmn-9vu6t-SCl6HQrzbh";
    const PROD_INPUT_DATA: &[u8] = include_bytes!("../assets/prod_update_data.json");

    #[test]
    fn test_verify_succeeds_if_valid() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert!(r.is_ok(), "Found unexpected error: {}", r.unwrap_err());
    }

    #[test]
    fn test_verify_fails_if_root_hash_mismatch() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            &ROOT_HASH.replace("A", "B"),
            VALID_HOSTNAME,
        );
        assert_eq!(r, Err(BalrogError::RootHashMismatch));
    }

    #[test]
    fn test_verify_fails_if_der_invalid() {
        let r = parse_and_verify(
            INVALID_CERTIFICATE_DER,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert!(
            matches!(r, Err(BalrogError::X509(X509Error::Der(_)))),
            "Found unexpected error: {}",
            r.unwrap_err()
        );
    }

    #[test]
    fn test_verify_fails_if_cert_empty() {
        let pem: &[u8] = b"\
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----";

        let r = parse_and_verify(
            pem,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert!(
            matches!(r, Err(BalrogError::X509(X509Error::Der(_)))),
            "Found unexpected error: {}",
            r.unwrap_err()
        );
    }

    #[test]
    fn test_verify_fails_if_pem_empty() {
        let pem: &[u8] = &[];
        let r = parse_and_verify(
            pem,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert!(
            matches!(r, Err(BalrogError::CertificateNotFound)),
            "Found unexpected error: {}",
            r.unwrap_err()
        );
    }

    #[test]
    fn test_verify_fails_if_pem_wrong_type() {
        // Generated from: openssl ecparam -name secp384k1 -genkey
        let pem: &[u8] = b"\
-----BEGIN EC PRIVATE KEY-----
MIGkAgEBBDD+fvOhk1l7iyXF5OztCR0hFYSWFivpOu9MIBX9RMm7G7t+PTbQGzWQ
Qtcp9raswDugBwYFK4EEACKhZANiAATNdZWfgxAxGgbVNBwC8TbsFgm+RNBhZnUa
cL9WgG8LqAoCip698cJfLm7TVO4LKv8MtfA1wWm/H5M3v9jRMNg9dsDf0j4fTefd
W6AQ6dHMhqgvSiqCVn1t04dFPyqczNI=
-----END EC PRIVATE KEY-----;";
        let r = parse_and_verify(
            pem,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert_eq!(r, Err(BalrogError::X509(X509Error::InvalidCertificate)));
    }

    #[test]
    fn test_verify_fails_if_hostname_mismatch() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            ROOT_HASH,
            "example.com",
        );
        assert_eq!(r, Err(BalrogError::HostnameMismatch));
    }

    #[test]
    fn test_verify_fails_if_cert_has_expired() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            VALID_SIGNATURE,
            1215559719, // July 9, 2008
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert_eq!(r, Err(BalrogError::CertificateExpired));
    }

    #[test]
    fn test_verify_fails_if_chain_out_of_order() {
        // Parse the chain then swap the leaf and intermediate certs.
        let mut pem: Vec<Pem> = match parse_pem_chain(VALID_CERT_CHAIN) {
            Err(e) => panic!("Found unexpected error: {}", e),
            Ok(x) => x,
        };
        let leaf = pem.remove(0);
        pem.insert(1, leaf);

        let b = match Balrog::new(&pem) {
            Err(e) => panic!("Found unexpected error: {}", e),
            Ok(x) => x,
        };
        let r = b.verify_chain(VALID_TIMESTAMP, ROOT_HASH);
        assert_eq!(r, Err(BalrogError::ChainSubjectMismatch));
    }

    #[test]
    fn test_verify_fails_if_bad_chain_signature() {
        let mut pem: Vec<Pem> = match parse_pem_chain(VALID_CERT_CHAIN) {
            Err(e) => panic!("Found unexpected error: {}", e),
            Ok(x) => x,
        };
        // An ASN.1 X509 certificate will take the form of:
        // Certificate ::= SEQUENCE {
        //     tbsCertificate     TBSCertificate,
        //     signatureAlgorithm AlgorithmIdentifier,
        //     signature          BITSTRING
        // }
        //
        // This means the signature will always reside in the trailing bytes of
        // the DER-encoded certificate, and we should be able to corrupt it by
        // flipping a few bits.
        let tail = pem[1].contents.pop().unwrap();
        pem[1].contents.push(tail ^ 0x80);

        let b = match Balrog::new(&pem) {
            Err(e) => panic!("Found unexpected error: {}", e),
            Ok(x) => x,
        };
        let r = b.verify_chain(VALID_TIMESTAMP, ROOT_HASH);
        assert_eq!(
            r,
            Err(BalrogError::from(X509Error::SignatureVerificationError))
        );
    }

    #[test]
    fn test_verify_fails_if_bad_content_signature() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            &VALID_SIGNATURE.to_ascii_lowercase(), // altering case should modify the base64 signature without changing its length.
            VALID_TIMESTAMP,
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert_eq!(
            r,
            Err(BalrogError::from(X509Error::SignatureVerificationError))
        );
    }

    #[test]
    fn test_everything_fails_without_init() {
        let b = Balrog {
            chain: Vec::new(),
            root_hash: Vec::new(),
        };

        let r = b.verify_chain(
            1615559719, // March 12, 2021
            ROOT_HASH,
        );
        assert_eq!(r, Err(BalrogError::CertificateNotFound));

        let r = b.verify_leaf_hostname(VALID_HOSTNAME);
        assert_eq!(r, Err(BalrogError::CertificateNotFound));

        let r = b.verify_content_signature(VALID_INPUT, VALID_SIGNATURE);
        assert_eq!(r, Err(BalrogError::CertificateNotFound));
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
        );
        assert!(r, "Verification failed via FFI");

        // Retake pointers for garbage collection.
        unsafe {
            let _ = CString::from_raw(prod_signature_cstr);
            let _ = CString::from_raw(prod_root_hash_cstr);
            let _ = CString::from_raw(prod_hostname_cstr);
        };
    }

    /* TODO: We could use tests for:
     *  - Missing code signing permissions.
     *  - Chains of irregular length.
     *  - Other signature algorithms.
     */
}
