/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* This module handles verification of an X509 certificate chain and Mozilla's
 * content signature algorithm which is used for signing and authentication of
 * updates.
 *
 * This provides methods to parse an X509 certificate chain out of a PEM file,
 * perform certificate chain validation to ensure the code signing certificate
 * can be trusted, and verifies a content signature against the code signing
 * certificate.
 *
 * The content signature itself is a base64-encoded ECDSA signature, using the
 * content prefixed with "Content-Signature\x00" as the input data.
 */

use hex;
use asn1_rs::ToDer;
use ring::digest;
use x509_parser::prelude::*;

use oid_registry::{
    OID_SIG_ECDSA_WITH_SHA256,
    OID_SIG_ECDSA_WITH_SHA384,
};

pub struct Balrog<'a> { 
    pub chain: Vec<X509Certificate<'a>>,
    pub root_hash: Vec<u8>
}

// Errors that can be returned from the Balrog module.
#[derive(Clone, Debug, PartialEq, thiserror::Error)]
pub enum BalrogError {
    #[error("Chain subject/issuer mismatch")]
    ChainSubjectMismatch,
    #[error("Certificate expired")]
    CertificateExpired,
    #[error("Issuer unauthorized for certificate signing")]
    IssuerUnauthorized,
    #[error("Certificate unauthorized for code signing")]
    CodeSignUnauthorized,
    #[error("Root hash parse failed")]
    RootHashParseFailed,
    #[error("Root hash mismatch")]
    RootHashMismatch,
    #[error("Hostname mismatch")]
    HostnameMismatch,
    #[error("Signature decoding failed")]
    SignatureDecodeError,

    #[error("PEM decoding error: {0:?}")]
    PemDecodeError(String),
    #[error("X509 validation failed: {0:?}")]
    X509ValidationError(String),
    #[error("X509 error: {0:?}")]
    X509(X509Error)
}

impl From<X509Error> for BalrogError {
    fn from(e: X509Error) -> Self {
        BalrogError::X509(e)
    }
}

impl From<PEMError> for BalrogError {
    fn from(e: PEMError) -> Self {
        BalrogError::PemDecodeError(e.to_string())
    }
}

pub fn parse_pem_chain(input: &[u8]) -> Result<Vec<Pem>, PEMError> {
    Pem::iter_from_buffer(input)
        .collect::<Result<Vec<Pem>, PEMError>>()
}

/* A one-and-done function that parses a signature chain and validates a content signature. */
pub fn parse_and_verify(
    x5u: &[u8],
    input: &[u8],
    signature: &str,
    current_time: i64,
    root_hash: &str,
    leaf_subject: &str
) -> Result<(), BalrogError> {
    let pem_chain = match parse_pem_chain(x5u) {
        Err(e) => return Err(BalrogError::from(e)),
        Ok(x) => x
    };

    let balrog = match Balrog::new(&pem_chain) {
        Err(e) => return Err(BalrogError::from(e)),
        Ok(x) => x
    };

    let _ = match balrog.verify_chain(current_time, root_hash) {
        Err(e) => return Err(BalrogError::from(e)),
        Ok(x) => x
    };

    let _ = match balrog.verify_leaf_hostname(leaf_subject) {
        Err(e) => return Err(e),
        Ok(x) => x
    };

    let _ = match balrog.verify_content_signature(input, signature) {
        Err(e) => return Err(BalrogError::from(e)),
        Ok(x) => x
    };

    Ok(())
}

impl<'a> Balrog<'_> {
    pub fn new(list: &'a Vec<Pem>) -> Result<Balrog<'a>, X509Error> {
        if list.is_empty() {
            return Err(X509Error::InvalidCertificate);
        }

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
        *  1. The issuer name in cert N matches the subject name of cert N+1.
        *  2. The validity time of cert N contains the current time.
        *  3. The cert N+1 contains a key usage extension permitting CA.
        *  4. Extract the public key from cert N+1
        *  5. Call X509Certificate::verify_signature with the key from step 4.
        *
        * For the last certificate in the chain (the root), repeat the same
        * steps using the last certificate as both cert N and cert N+1, and
        * check that the SHA256 of the root certificate matches the root_hash
        * parameter to this function.
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
            Err(_e) => return Err(BalrogError::RootHashParseFailed),
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
        /* Check that the subject and issuer names match. */
        if subject.issuer() != issuer.subject() {
            return Err(BalrogError::ChainSubjectMismatch);
        }

        /* Check the certificate validity period. */
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

        /* Check X509 certificate validation. */
        let mut logger = VecLogger::default();
        // structure validation status
        let ok = X509StructureValidator
            .chain(X509CertificateValidator)
            .validate(subject, &mut logger);
        if !ok {
            let reason = match logger.errors().first() {
                None => "Unknown error",
                Some(s) => s
            };
            return Err(BalrogError::X509ValidationError(reason.to_string()));
        }
        
        /* Verify the cryptographic signature. */
        subject.verify_signature(Some(issuer.public_key()))
            .map_err(|e| { BalrogError::from(e) })
    }

    /* Check a hostname against the leaf certificate. */
    pub fn verify_leaf_hostname(&self, hostname: &str) -> Result<(), BalrogError> {
        assert!(!self.chain.is_empty());
        let leaf = self.chain.first().unwrap();

        /* First check, does the hostname match one of the subject common names? */
        for cn in leaf.subject().iter_common_name() {
            let name = match cn.as_str() {
                Err(e) => return Err(BalrogError::from(e)),
                Ok(x) => x
            };
            if name == hostname {
                return Ok(());
            }
        }

        /* Second check, does the hostname match one of the subject alternative names? */
        let san = match leaf.subject_alternative_name() {
            Err(e) => return Err(BalrogError::from(e)),
            Ok(x) => x
        };
        if san.is_some() {
            let as_dns_name = GeneralName::DNSName(hostname);
            for x in san.unwrap().value.general_names.iter() {
                if x == &as_dns_name {
                    return Ok(());
                }
            }
        }

        /* Otherwise, there is no match! */
        Err(BalrogError::HostnameMismatch)
    }

    /* Take a fixed-length ECDSA signature and convert into ASN.1 DER encoding */
    fn ecdsa_signature_to_asn1(input: &[u8]) -> Result<Vec<u8>, BalrogError> {
        /* ECDSA signatures take the ASN.1 form of:
         * SEQUENCE {
         *   r INTEGER,
         *   s INTEGER
         * }
         */
        /* Cut the fixed signature into component r and s values. */
        let (r, s) = input.split_at(input.len() / 2);
        let mut r = r.to_vec();
        let mut s = s.to_vec();

        /* ASN.1 integers are signed, but ECDSA signature points are always unsigned.
         * If the leading bit is zero, we must append an extra leading zero byte to
         * ensure the sign is positive.
         */
        if r[0] & 0x80 != 0 {
            r.insert(0, 0x00);
        }
        if s[0] & 0x80 != 0 {
            s.insert(0, 0x00);
        }

        /* Encode into an ASN.1 sequence. */
        let vec = vec![asn1_rs::Integer::new(&r), asn1_rs::Integer::new(&s)];
        let seq = match asn1_rs::Sequence::from_iter_to_der(vec.iter()) {
            Err(_e) => return Err(BalrogError::SignatureDecodeError),
            Ok(x) => x
        };
        
        /* Encode it. */
        seq.to_der_vec().map_err(|_e| BalrogError::SignatureDecodeError)
    }

    /* Verify a content signature against the certificate chain. */
    pub fn verify_content_signature(
        &self,
        input: &[u8],
        signature: &str
    ) -> Result<(), BalrogError> {
        assert!(!self.chain.is_empty());

        /* To be authorized for code signing, every certificate in the chain needs the
         * code signing bit in the extended key usage to be set. */
        for x in self.chain.iter() {
            let code_sign_ext = match x.extended_key_usage() {
                Err(_e) => false,
                Ok(None) => false,
                Ok(x) => x.unwrap().value.code_signing
            };
            if !code_sign_ext {
                return Err(BalrogError::CodeSignUnauthorized);
            }
        }

        /*  To verify the code signature, we must:
         *  1. Parse the signature into an asn1_rs::asn1_types::bitstring::Bitstring
         *  2. Extract the public key from the leaf certificate.
         *  3. Call x509_parser::verify::verify_signature(
         *       public_key,
         *       public_key.algothim,
         *       signature_bitstring,
         *       input
         *     )
         */
        
        /* Parse the signature into an ASN1 bitstring. */
        let sig_decode = match data_encoding::BASE64URL_NOPAD.decode(signature.as_bytes()) {
            Err(_e) => return Err(BalrogError::SignatureDecodeError),
            Ok(x) => x
        };
        let sig_der = match Self::ecdsa_signature_to_asn1(sig_decode.as_slice()) {
            Err(e) => return Err(e),
            Ok(x) => x
        };
        let sig_asn1 = asn1_rs::BitString::new(0, sig_der.as_slice());
        let pubkey = self.chain.first().unwrap().public_key();
        
        /*
         * TODO: Should this be determined by looking at the public key? Or is it
         * safe to assume an elliptic key with SHA384?
         *
         * The rc_crypto crate assumes NIST P-384/SHA384 everywhere.
         * The golang library chooses a SHA2 that matches the pubkey key size.
         * Do we care about the RSA/DSA ecosystem?
         */
        let algorithm = AlgorithmIdentifier::new(OID_SIG_ECDSA_WITH_SHA384, None);

        /* Append the Content-Signature prefix. */
        let prefix: &[u8] = b"Content-Signature:\x00";
        let message = [prefix, input].concat();

        x509_parser::verify::verify_signature(&pubkey, &algorithm, &sig_asn1, &message)
            .map_err(|e| { BalrogError::from(e) })
    }
}
