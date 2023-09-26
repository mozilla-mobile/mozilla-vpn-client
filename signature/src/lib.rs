/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use ring::signature;
use std::os::raw::c_uchar;
use x509_parser::prelude::*;

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

#[no_mangle]
pub extern "C" fn verify_balrog(
    x5u_ptr: *const c_uchar,
    x5u_length: usize,
    input: *const c_uchar,
    signature: *const c_uchar,
    root_hash: *const c_uchar,
    leaf_subject: *const c_uchar
) -> bool {
    let x5u_raw = unsafe { std::slice::from_raw_parts(x5u_ptr, x5u_length) };
    
    /* Parse the certificate chain. */
    let pem_chain: Vec<Pem> = match Pem::iter_from_buffer(x5u_raw)
        .collect::<Result<Vec<Pem>, PEMError>>() {
            Err(e) => {
                eprintln!("{}", e);
                return false
            }
            Ok(x) => x
        };
    let x5u_chain: Vec<Result<X509Certificate, X509Error>> = pem_chain.iter()
        .map(|x| { x.parse_x509() }).collect();

    /* Verify the certificate chain. */
    verify_cert_chain(&x5u_chain, root_hash)

    /* TODO: Verify the leaf certificate. */

    /* TODO: Verify the code signature. To do this, we must:
     *  1. Parse the signature into an asn1_rs::asn1_types::bitstring::Bitstring
     *  2. Extract the public key from the leaf certificate.
     *  3. Call x509_parser::verify::verify_signature(
     *       public_key,
     *       public_key.algothim,
     *       signature_bitstring,
     *       input
     *     )
     */
}

/* Cryptographically verify the signatures in the certificate chain. */
fn verify_cert_chain(
    chain: &Vec<X509Certificate>,
    root_hash: *const c_uchar
) -> bool {
    /* For each certificate N in the chain, check that:
     *  1. The issuer hash in cert N matches the subject hash of cert N+1.
     *  2. The issuer name in cert N matches the subject name of cert N+1.
     *  3. The validity time of cert N contains the current time.
     *  4. The cert N+1 contains a key usage extension permitting CA.
     *  5. Extract the public key from cert N+1
     *  6. Call X509Certificate::verify_signature with the key from step 5.
     *
     * For the last certificate in the chain, Repeat the same steps using
     * the last certificate as both cert N and cert N+1. And additionally
     * check that the subject has matches the root_hash parameter to this
     * function.
     *
     * This could probably benefit from a helper function of the form
     * fn verify_cert_sig(
     *     subject: &X509Certificate,
     *     issuer: &X509Certificate
     * ) -> bool {...}
     */
    true
}

fn verify_leaf_cert(
    leaf_cert: &X509Certificate,
    leaf_subject: *const c_uchar
) -> bool {
    /* For the leaf certificate, we really just want to ensure that the
     * subject name matches the hostname we got from Balrog, and then
     * check that the certificate contains the code signing extension.
     */
    true
}