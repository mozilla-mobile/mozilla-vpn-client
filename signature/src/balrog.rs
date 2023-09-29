/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use hex;
use ring::digest;
use ring::signature;
use std::os::raw::c_uchar;
use thiserror::Error;
use x509_parser::prelude::*;

pub struct Balrog<'a> { 
    pub chain: Vec<X509Certificate<'a>>,
    pub root_hash: Vec<u8>
}

// Errors that can be returned from the Balrog module.
#[derive(Clone, Debug, thiserror::Error)]
pub enum BalrogError {
    #[error("Chain subject/issuer mismatch")]
    ChainSubjectMismatch,
    #[error("Certificate expired")]
    CertificateExpired,
    #[error("Issuer unauthorized for certificate signing")]
    IssuerUnauthorized,
    #[error("Root hash parse failed")]
    RootHashParseFailed,
    #[error("Root hash mismatch")]
    RootHashMismatch,

    #[error("nom error: {0:?}")]
    X509(X509Error)
}

impl From<X509Error> for BalrogError {
    fn from(e: X509Error) -> Self {
        BalrogError::X509(e)
    }
}

pub fn parse_pem_chain(input: &[u8]) -> Result<Vec<Pem>, PEMError> {
    Pem::iter_from_buffer(input)
        .collect::<Result<Vec<Pem>, PEMError>>()
}

impl<'a> Balrog<'_> {
    pub fn new(list: &'a Vec<Pem>) -> Result<Balrog<'a>, X509Error> {
        let mut parsed: Vec<X509Certificate<'a>> = Vec::new();
        let mut hash: Vec<u8> = Vec::new();
        for (i, pem) in list.iter().enumerate() {
            if pem.label != "CERTIFICATE" {
                return Err(X509Error::InvalidCertificate);
            }

            let x509 = match pem.parse_x509() {
                Err(e) => return Err(X509Error::from(e)),
                Ok(x) => x
            };
            parsed.push(x509);

            /* For the final certificate, calculate its SHA256 hash. */
            /* TODO: Technically, we can get more bytes in the PEM beyond the end
             * of the ASN.1 certificate sequence, and this function will include
             * them in the hash. This feels like an unclear edge case though.
             */
            if i+1 == list.len() {
                let digest = digest::digest(&digest::SHA256, pem.contents.as_slice());
                hash.extend_from_slice(digest.as_ref());
            }
        }

        Ok(Balrog{chain: parsed, root_hash: hash})
    }

    /* Ensure that the certificate chain is valid. */
    pub fn verify_chain(
        &self,
        current_time: i64,
        root_hash: &str
    ) -> Result<(), BalrogError> {
        assert!(!self.chain.is_empty());
        assert!(!self.root_hash.is_empty());

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
        * TODO: Do we need to run anything through the x509_parser validate
        * module, or are these checks here sufficient?
        */
        let mut index = 0;
        while index+1 < self.chain.len() {
            let subject = &self.chain[index];
            let issuer = &self.chain[index+1];
            let _ = match Self::verify_cert_chain_pair(subject, issuer, current_time) {
                Err(e) => return Err(e),
                Ok(x) => x
            };
            index = index+1;
        }

        /* Verify the root certificate. */
        let root_cert = self.chain.last().unwrap();
        let _ = match Self::verify_cert_chain_pair(root_cert, root_cert, current_time) {
            Err(e) => return Err(e),
            Ok(x) => x
        };

        let hash_stripped = root_hash.replace(":", "");
        let hash_decoded = match hex::decode(hash_stripped) {
            Err(e) => return Err(BalrogError::RootHashParseFailed),
            Ok(x) => x
        };
        if self.root_hash != hash_decoded {
            return Err(BalrogError::RootHashMismatch);
        }
        
        /* Success! */
        Ok(())
    }

    /* Internal helper function to check that a certificate was signed by its issuer. */
    fn verify_cert_chain_pair(
        subject: &X509Certificate,
        issuer: &X509Certificate,
        current_time: i64,
    ) -> Result<(), BalrogError> {
        /* TODO: Check the subject and issuer key hashes. */
        /* Annoyingly, these are extensions and require some parsing. */

        /* Check that the subject and issuer names match. */
        if subject.issuer() != issuer.subject() {
            return Err(BalrogError::ChainSubjectMismatch);
        }

        /* Check the certificate validitiy period. */
        let asn_timestamp = ASN1Time::from_timestamp(current_time).unwrap();
        if !subject.validity().is_valid_at(asn_timestamp) {
            return Err(BalrogError::CertificateExpired);
        }
        if !issuer.validity().is_valid_at(asn_timestamp) {
            return Err(BalrogError::CertificateExpired);
        }

        /* Check that the issuer is permitted to sign certificates. */
        let ca_key_usage = match issuer.key_usage() {
            Err(e) => return Err(BalrogError::from(e)),
            Ok(x) => x
        };
        let ca_key_usage = ca_key_usage.unwrap().value;
        if !ca_key_usage.key_cert_sign() {
            return Err(BalrogError::IssuerUnauthorized);
        }
        if !issuer.is_ca() {
            return Err(BalrogError::IssuerUnauthorized);
        }

        /* Verify the cryptographic signature. */
        subject.verify_signature(Some(issuer.public_key()))
            .map_err(|e| { BalrogError::from(e) })
    }
}
