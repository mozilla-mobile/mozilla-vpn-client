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

use asn1_rs::ToDer;
use x509_parser::prelude::*;

use oid_registry::{
    OID_KEY_TYPE_EC_PUBLIC_KEY, OID_SIG_ECDSA_WITH_SHA256, OID_SIG_ECDSA_WITH_SHA384,
};

pub struct Balrog<'a> {
    pub chain: Vec<X509Certificate<'a>>,
}

// Errors that can be returned from the Balrog module.
#[derive(Clone, Debug, PartialEq, thiserror::Error)]
pub enum BalrogError {
    #[error("Certificate chain not found")]
    CertificateNotFound,
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
    X509(X509Error),
}

impl From<X509Error> for BalrogError {
    fn from(e: X509Error) -> Self {
        BalrogError::X509(e)
    }
}

impl From<asn1_rs::Err<X509Error>> for BalrogError {
    fn from(e: asn1_rs::Err<X509Error>) -> Self {
        BalrogError::X509(X509Error::from(e))
    }
}

impl From<PEMError> for BalrogError {
    fn from(e: PEMError) -> Self {
        BalrogError::PemDecodeError(e.to_string())
    }
}

pub fn parse_pem_chain(input: &[u8]) -> Result<Vec<Pem>, BalrogError> {
    Ok(Pem::iter_from_buffer(input).collect::<Result<Vec<Pem>, PEMError>>()?)
}

impl<'a> Balrog<'_> {
    pub fn new(list: &'a Vec<Pem>) -> Result<Balrog<'a>, BalrogError> {
        if list.is_empty() {
            return Err(BalrogError::CertificateNotFound);
        }

        let mut parsed: Vec<X509Certificate<'a>> = Vec::new();
        for pem in list {
            if pem.label != "CERTIFICATE" {
                return Err(BalrogError::from(X509Error::InvalidCertificate));
            }

            /* Parse the X509 certificate. */
            let (tail, x509) = X509Certificate::from_der(&pem.contents)?;
            if !tail.is_empty() {
                return Err(BalrogError::from(X509Error::InvalidCertificate));
            }
            parsed.push(x509);
        }

        Ok(Balrog {
            chain: parsed,
        })
    }

    /* Ensure that the certificate chain is valid. */
    pub fn verify_chain(&self, current_time: i64) -> Result<(), BalrogError> {
        if self.chain.is_empty() {
            return Err(BalrogError::CertificateNotFound);
        }

        /* For each certificate N in the chain, check that:
         *  1. The issuer name in cert N matches the subject name of cert N+1.
         *  2. The validity time of cert N contains the current time.
         *  3. The cert N+1 contains a key usage extension permitting CA.
         *  4. Extract the public key from cert N+1
         *  5. Call X509Certificate::verify_signature with the key from step 4.
         *
         * For the last certificate in the chain (the root), repeat the same
         * steps using the last certificate as both cert N and cert N+1.
         */
        let mut index = 0;
        while index + 1 < self.chain.len() {
            let subject = &self.chain[index];
            let issuer = &self.chain[index + 1];
            Self::verify_cert_chain_pair(subject, issuer, current_time)?;
            index = index + 1;
        }

        /* Verify the root certificate. */
        let root_cert = self.chain.last().unwrap();
        Self::verify_cert_chain_pair(root_cert, root_cert, current_time)?;

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
        let asn_timestamp = ASN1Time::from_timestamp(current_time)?;
        if !subject.validity().is_valid_at(asn_timestamp) {
            return Err(BalrogError::CertificateExpired);
        }
        if !issuer.validity().is_valid_at(asn_timestamp) {
            return Err(BalrogError::CertificateExpired);
        }

        /* Check that the issuer is permitted to sign certificates. */
        let Some(ca_key_usage) = issuer.key_usage()? else {
            return Err(BalrogError::IssuerUnauthorized);
        };
        if !ca_key_usage.value.key_cert_sign() {
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
                None => String::from("Unknown error"),
                Some(s) => s.to_string(),
            };
            return Err(BalrogError::X509ValidationError(reason));
        }

        /* Verify the cryptographic signature. */
        Ok(subject.verify_signature(Some(issuer.public_key()))?)
    }

    /* Check a hostname against the leaf certificate. */
    pub fn verify_leaf_hostname(&self, hostname: &str) -> Result<(), BalrogError> {
        let Some(leaf) = self.chain.first() else {
            return Err(BalrogError::CertificateNotFound);
        };

        /* First check, does the hostname match one of the subject common names? */
        for cn in leaf.subject().iter_common_name() {
            let name = cn.as_str()?;
            if name == hostname {
                return Ok(());
            }
        }

        /* Second check, does the hostname match one of the subject alternative names? */
        let as_dns_name = GeneralName::DNSName(hostname);
        if let Some(ext) = leaf.subject_alternative_name()? {
            for san in ext.value.general_names.iter() {
                if san == &as_dns_name {
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
        let Ok(seq) = asn1_rs::Sequence::from_iter_to_der(vec.iter()) else {
            return Err(BalrogError::SignatureDecodeError);
        };

        /* Encode it. */
        seq.to_der_vec()
            .map_err(|_e| BalrogError::SignatureDecodeError)
    }

    /* Verify a content signature against the certificate chain. */
    pub fn verify_content_signature(
        &self,
        input: &[u8],
        signature: &str,
    ) -> Result<(), BalrogError> {
        /* To be authorized for code signing, every certificate in the chain needs the
         * code signing bit in the extended key usage to be set. */
        for x in self.chain.iter() {
            let code_sign_ext = match x.extended_key_usage() {
                Err(_e) => false,
                Ok(None) => false,
                Ok(Some(x)) => x.value.code_signing,
            };
            if !code_sign_ext {
                return Err(BalrogError::CodeSignUnauthorized);
            }
        }

        let Some(leaf) = self.chain.first() else {
            return Err(BalrogError::CertificateNotFound);
        };

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
        let Ok(sig_decode) = data_encoding::BASE64URL_NOPAD.decode(signature.as_bytes()) else {
            return Err(BalrogError::SignatureDecodeError);
        };
        let sig_der = Self::ecdsa_signature_to_asn1(sig_decode.as_slice())?;
        let sig_asn1 = asn1_rs::BitString::new(0, sig_der.as_slice());
        let pubkey = leaf.public_key();

        /* Select the signature algorithm according to the public key size.
         *
         * Note that there exists some disagreement in the algorithm to choose
         * here. While Firefox and Gecko assume P384/SHA384 everywhere, but we
         * follow the implementation of the Golang library.
         *
         * TODO: P-521/SHA512 is supported by the Golang implementation, and
         * would correspond to OID_SIG_ECDSA_WITH_SHA512, but x509_parser and
         * ring::signature don't support it yet. In this case we'll go with
         * SHA384 and hope for the best.
         */
        if pubkey.algorithm.algorithm != OID_KEY_TYPE_EC_PUBLIC_KEY {
            return Err(BalrogError::X509(X509Error::SignatureUnsupportedAlgorithm));
        }
        let keybits = pubkey.parsed()?.key_size();
        let algorithm = if keybits <= 256 {
            AlgorithmIdentifier::new(OID_SIG_ECDSA_WITH_SHA256, None)
        } else {
            AlgorithmIdentifier::new(OID_SIG_ECDSA_WITH_SHA384, None)
        };

        /* Append the Content-Signature prefix. */
        let prefix: &[u8] = b"Content-Signature:\x00";
        let message = [prefix, input].concat();

        Ok(x509_parser::verify::verify_signature(
            &pubkey, &algorithm, &sig_asn1, &message,
        )?)
    }

    /* Wrapper function to verify everything at once. */
    pub fn verify(
        &self,
        input: &[u8],
        signature: &str,
        current_time: i64,
        leaf_subject: &str,
    ) -> Result<(), BalrogError> {
        /* Verify things. */
        self.verify_chain(current_time)?;
        self.verify_leaf_hostname(leaf_subject)?;
        self.verify_content_signature(input, signature)?;
    
        /* Success */
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    // Copied from https://github.com/mozilla/application-services/blob/main/components/support/rc_crypto/src/contentsignature.rs
    const VALID_CERT_CHAIN: &[u8] = include_bytes!("../assets/valid_cert_chain.pem");
    const VALID_INPUT: &[u8] = b"{\"data\":[],\"last_modified\":\"1603992731957\"}";
    const VALID_SIGNATURE: &str = "fJJcOpwdnkjEWFeHXfdOJN6GaGLuDTPGzQOxA2jn6ldIleIk6KqMhZcy2GZv2uYiGwl6DERWwpaoUfQFLyCAOcVjck1qlaaEFZGY1BQba9p99xEc9FNQ3YPPfvSSZqsw";
    const VALID_HOSTNAME: &str = "remote-settings.content-signature.mozilla.org";
    const VALID_TIMESTAMP: i64 = 1615559719; // March 12, 2021

    const BOGUS_DATA: &str =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor
incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis
nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu
fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in
culpa qui officia deserunt mollit anim id est laborum.";

    /* A one-and-done function that parses a signature chain and validates a content signature. */
    #[cfg(test)]
    fn parse_and_verify(
        x5u: &[u8],
        input: &[u8],
        signature: &str,
        current_time: i64,
        leaf_subject: &str,
    ) -> Result<(), BalrogError> {
        /* Parse the certificate chain. */
        let pem_chain = parse_pem_chain(x5u)?;
        let balrog = Balrog::new(&pem_chain)?;

        balrog.verify(input, signature, current_time, leaf_subject)
    }

    #[test]
    fn test_verify_succeeds_if_valid() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
            VALID_HOSTNAME,
        );
        assert!(r.is_ok(), "Found unexpected error: {}", r.unwrap_err());
    }

    #[test]
    fn test_verify_fails_if_der_invalid() {
        let r = parse_and_verify(
            include_bytes!("../assets/invalid_der_content.pem"),
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
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
            VALID_HOSTNAME,
        );
        assert!(
            matches!(r, Err(BalrogError::CertificateNotFound)),
            "Found unexpected error: {}",
            r.unwrap_err()
        );
    }

    #[test]
    fn test_pem_ignores_bogus_data() {
        // Data before or after any PEM block should be ignored.
        let pem = parse_pem_chain(BOGUS_DATA.as_bytes()).unwrap();
        assert!(pem.is_empty());
    }

    #[test]
    fn test_pem_fails_on_bogus_content() {
        let bogus_chain = [
            b"-----BEGIN CERTIFICATE-----\n",
            BOGUS_DATA.as_bytes(),
            b"\n-----END CERTIFICATE-----",
        ]
        .concat();

        match parse_pem_chain(&bogus_chain) {
            Err(e) => assert!(matches!(e, BalrogError::PemDecodeError(_))),
            Ok(_) => panic!("Failed to detect bogus PEM content."),
        };
    }

    #[test]
    fn test_pem_fails_on_unclosed_chain() {
        let unclosed_chain = VALID_CERT_CHAIN.split_at(VALID_CERT_CHAIN.len() - 20).0;
        match parse_pem_chain(unclosed_chain) {
            Err(e) => assert!(matches!(e, BalrogError::PemDecodeError(_))),
            Ok(_) => panic!("Failed to detect unclosed PEM content."),
        };
    }

    #[test]
    fn test_verify_signature_fails_on_bogus_signature() {
        let pem = parse_pem_chain(VALID_CERT_CHAIN).unwrap();
        let b = Balrog::new(&pem).unwrap();

        assert_eq!(
            b.verify_content_signature(VALID_INPUT, BOGUS_DATA),
            Err(BalrogError::SignatureDecodeError)
        );
    }

    #[test]
    fn test_new_fails_if_vec_empty() {
        let pem: Vec<Pem> = Vec::new();
        match Balrog::new(&pem) {
            Err(e) => assert_eq!(e, BalrogError::CertificateNotFound),
            Ok(_) => panic!("Failed to return error on empty PEM vector"),
        }
    }

    #[test]
    fn test_new_fails_on_trailing_data() {
        let mut pem = parse_pem_chain(VALID_CERT_CHAIN).unwrap();
        pem[1].contents.push(0x00);

        match Balrog::new(&pem) {
            Err(e) => assert_eq!(e, BalrogError::X509(X509Error::InvalidCertificate)),
            Ok(_) => panic!("Failed to return error on trailing X509 certificate data"),
        }
    }

    #[test]
    fn test_verify_fails_if_pem_wrong_type() {
        let pem: &[u8] = include_bytes!("../assets/test_private_key.pem");
        let r = parse_and_verify(
            pem,
            VALID_INPUT,
            VALID_SIGNATURE,
            VALID_TIMESTAMP,
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
        let r = b.verify_chain(VALID_TIMESTAMP);
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
        let r = b.verify_chain(VALID_TIMESTAMP);
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
            VALID_HOSTNAME,
        );
        assert_eq!(
            r,
            Err(BalrogError::from(X509Error::SignatureVerificationError))
        );
    }

    #[test]
    fn test_verify_signature_fails_on_missing_codesign() {
        let pem = parse_pem_chain(include_bytes!("../assets/missing_codesign.pem")).unwrap();
        let b = Balrog::new(&pem).unwrap();

        assert_eq!(
            b.verify_content_signature(VALID_INPUT, VALID_SIGNATURE),
            Err(BalrogError::CodeSignUnauthorized)
        );
    }

    #[test]
    fn test_verify_signature_fails_on_rsa_leaf() {
        let pem = parse_pem_chain(include_bytes!("../assets/rsa_leaf_cert.pem")).unwrap();
        let b = Balrog::new(&pem).unwrap();

        assert_eq!(
            b.verify_content_signature(VALID_INPUT, VALID_SIGNATURE),
            Err(BalrogError::X509(X509Error::SignatureUnsupportedAlgorithm))
        );
    }

    #[test]
    fn test_everything_fails_without_init() {
        let b = Balrog {
            chain: Vec::new(),
        };

        let r = b.verify_chain(VALID_TIMESTAMP);
        assert_eq!(r, Err(BalrogError::CertificateNotFound));

        let r = b.verify_leaf_hostname(VALID_HOSTNAME);
        assert_eq!(r, Err(BalrogError::CertificateNotFound));

        let r = b.verify_content_signature(VALID_INPUT, VALID_SIGNATURE);
        assert_eq!(r, Err(BalrogError::CertificateNotFound));
    }
}
