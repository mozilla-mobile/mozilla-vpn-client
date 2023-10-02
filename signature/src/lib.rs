/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use ring::signature;
use std::os::raw::c_uchar;
use std::ffi::CStr;
use x509_parser::prelude::*;
use crate::balrog::*;

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

#[no_mangle]
pub extern "C" fn verify_balrog(
    x5u_ptr: *const c_uchar,
    x5u_length: usize,
    input_ptr: *const c_uchar,
    input_length: usize,
    signature: *const i8,
    root_hash: *const i8,
    leaf_subject: *const i8
) -> bool {
    /* Translate C/FFI arguments into Rust types. */
    let x5u = unsafe { std::slice::from_raw_parts(x5u_ptr, x5u_length) };
    let input = unsafe { std::slice::from_raw_parts(input_ptr, input_length) };
    let sig_str = match unsafe { CStr::from_ptr(signature) }.to_str() {
        Err(e) => {
            eprintln!("{}", e);
            return false
        }
        Ok(x) => x
    };
    let root_hash_str = match unsafe { CStr::from_ptr(root_hash) }.to_str() {
        Err(e) => {
            eprintln!("{}", e);
            return false
        }
        Ok(x) => x
    };
    let leaf_subect_str = match unsafe { CStr::from_ptr(leaf_subject) }.to_str() {
        Err(e) => {
            eprintln!("{}", e);
            return false
        }
        Ok(x) => x
    };
    let now: i64 = ASN1Time::now().timestamp();

    /* Perform the content signature validation. */
    let _ = match parse_and_verify(&x5u, &input, &sig_str, now, root_hash_str, leaf_subect_str) {
        Err(e) => {
            eprintln!("{}", e);
            return false
        }
        Ok(x) => x
    };

    /* Success! */
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

#[cfg(test)]
mod test {
    use super::*;

    const ROOT_HASH: &str = "3C:01:44:6A:BE:90:36:CE:A9:A0:9A:CA:A3:A5:20:AC:62:8F:20:A7:AE:32:CE:86:1C:B2:EF:B7:0F:A0:C7:45";
    const VALID_CERT_CHAIN: &[u8] = b"\
-----BEGIN CERTIFICATE-----
MIIDBjCCAougAwIBAgIIFml6g0ldRGowCgYIKoZIzj0EAwMwgaMxCzAJBgNVBAYT
AlVTMRwwGgYDVQQKExNNb3ppbGxhIENvcnBvcmF0aW9uMS8wLQYDVQQLEyZNb3pp
bGxhIEFNTyBQcm9kdWN0aW9uIFNpZ25pbmcgU2VydmljZTFFMEMGA1UEAww8Q29u
dGVudCBTaWduaW5nIEludGVybWVkaWF0ZS9lbWFpbEFkZHJlc3M9Zm94c2VjQG1v
emlsbGEuY29tMB4XDTIxMDIwMzE1MDQwNVoXDTIxMDQyNDE1MDQwNVowgakxCzAJ
BgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQHEw1Nb3VudGFp
biBWaWV3MRwwGgYDVQQKExNNb3ppbGxhIENvcnBvcmF0aW9uMRcwFQYDVQQLEw5D
bG91ZCBTZXJ2aWNlczE2MDQGA1UEAxMtcmVtb3RlLXNldHRpbmdzLmNvbnRlbnQt
c2lnbmF0dXJlLm1vemlsbGEub3JnMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE8pKb
HX4IiD0SCy+NO7gwKqRRZ8IhGd8PTaIHIBgM6RDLRyDeswXgV+2kGUoHyzkbNKZt
zlrS3AhqeUCtl1g6ECqSmZBbRTjCpn/UCpCnMLL0T0goxtAB8Rmi3CdM0cBUo4GD
MIGAMA4GA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAfBgNVHSME
GDAWgBQlZawrqt0eUz/t6OdN45oKfmzy6DA4BgNVHREEMTAvgi1yZW1vdGUtc2V0
dGluZ3MuY29udGVudC1zaWduYXR1cmUubW96aWxsYS5vcmcwCgYIKoZIzj0EAwMD
aQAwZgIxAPh43Bxl4MxPT6Ra1XvboN5O2OvIn2r8rHvZPWR/jJ9vcTwH9X3F0aLJ
9FiresnsLAIxAOoAcREYB24gFBeWxbiiXaG7TR/yM1/MXw4qxbN965FFUaoB+5Bc
fS8//SQGTlCqKQ==
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIF2jCCA8KgAwIBAgIEAQAAADANBgkqhkiG9w0BAQsFADCBqTELMAkGA1UEBhMC
VVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1Nb3VudGFpbiBWaWV3MRwwGgYDVQQK
ExNBZGRvbnMgVGVzdCBTaWduaW5nMSQwIgYDVQQDExt0ZXN0LmFkZG9ucy5zaWdu
aW5nLnJvb3QuY2ExMTAvBgkqhkiG9w0BCQEWInNlY29wcytzdGFnZXJvb3RhZGRv
bnNAbW96aWxsYS5jb20wHhcNMjEwMTExMDAwMDAwWhcNMjQxMTE0MjA0ODU5WjCB
ozELMAkGA1UEBhMCVVMxHDAaBgNVBAoTE01vemlsbGEgQ29ycG9yYXRpb24xLzAt
BgNVBAsTJk1vemlsbGEgQU1PIFByb2R1Y3Rpb24gU2lnbmluZyBTZXJ2aWNlMUUw
QwYDVQQDDDxDb250ZW50IFNpZ25pbmcgSW50ZXJtZWRpYXRlL2VtYWlsQWRkcmVz
cz1mb3hzZWNAbW96aWxsYS5jb20wdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARw1dyE
xV5aNiHJPa/fVHO6kxJn3oZLVotJ0DzFZA9r1sQf8i0+v78Pg0/c3nTAyZWfkULz
vOpKYK/GEGBtisxCkDJ+F3NuLPpSIg3fX25pH0LE15fvASBVcr8tKLVHeOmjggG6
MIIBtjAMBgNVHRMEBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAWBgNVHSUBAf8EDDAK
BggrBgEFBQcDAzAdBgNVHQ4EFgQUJWWsK6rdHlM/7ejnTeOaCn5s8ugwgdkGA1Ud
IwSB0TCBzoAUhtg0HE5Y0RNcmV/YQpjtFA8Z8l2hga+kgawwgakxCzAJBgNVBAYT
AlVTMQswCQYDVQQIEwJDQTEWMBQGA1UEBxMNTW91bnRhaW4gVmlldzEcMBoGA1UE
ChMTQWRkb25zIFRlc3QgU2lnbmluZzEkMCIGA1UEAxMbdGVzdC5hZGRvbnMuc2ln
bmluZy5yb290LmNhMTEwLwYJKoZIhvcNAQkBFiJzZWNvcHMrc3RhZ2Vyb290YWRk
b25zQG1vemlsbGEuY29tggRgJZg7MDMGCWCGSAGG+EIBBAQmFiRodHRwOi8vYWRk
b25zLmFsbGl6b20ub3JnL2NhL2NybC5wZW0wTgYDVR0eBEcwRaBDMCCCHi5jb250
ZW50LXNpZ25hdHVyZS5tb3ppbGxhLm9yZzAfgh1jb250ZW50LXNpZ25hdHVyZS5t
b3ppbGxhLm9yZzANBgkqhkiG9w0BAQsFAAOCAgEAtGTTzcPzpcdf07kIeRs9vPMx
qiF8ylW5L/IQ2NzT3sFFAvPW1vW1wZC0xAHMsuVyo+BTGrv+4mlD0AUR9acRfiTZ
9qyZ3sJbyhQwJAXLKU4YpnzuFOf58T/yOnOdwpH2ky/0FuHskMyfXaAz2Az4JXJH
TCgggqfdZNvsZ5eOnQlKoC5NadMa8oTI5sd4SyR5ANUPAtYok931MvVSz3IMbwTr
v4PPWXdl9SGXuOknSqdY6/bS1LGvC2KprsT+PBlvVtS6YgZOH0uCgTTLpnrco87O
ErzC2PJBA1Ftn3Mbaou6xy7O+YX+reJ6soNUV+0JHOuKj0aTXv0c+lXEAh4Y8nea
UGhW6+MRGYMOP2NuKv8s2+CtNH7asPq3KuTQpM5RerjdouHMIedX7wpNlNk0CYbg
VMJLxZfAdwcingLWda/H3j7PxMoAm0N+eA24TGDQPC652ZakYk4MQL/45lm0A5f0
xLGKEe6JMZcTBQyO7ANWcrpVjKMiwot6bY6S2xU17mf/h7J32JXZJ23OPOKpMS8d
mljj4nkdoYDT35zFuS1z+5q6R5flLca35vRHzC3XA0H/XJvgOKUNLEW/IiJIqLNi
ab3Ao0RubuX+CAdFML5HaJmkyuJvL3YtwIOwe93RGcGRZSKZsnMS+uY5QN8+qKQz
LC4GzWQGSCGDyD+JCVw=
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIHbDCCBVSgAwIBAgIEYCWYOzANBgkqhkiG9w0BAQwFADCBqTELMAkGA1UEBhMC
VVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1Nb3VudGFpbiBWaWV3MRwwGgYDVQQK
ExNBZGRvbnMgVGVzdCBTaWduaW5nMSQwIgYDVQQDExt0ZXN0LmFkZG9ucy5zaWdu
aW5nLnJvb3QuY2ExMTAvBgkqhkiG9w0BCQEWInNlY29wcytzdGFnZXJvb3RhZGRv
bnNAbW96aWxsYS5jb20wHhcNMjEwMjExMjA0ODU5WhcNMjQxMTE0MjA0ODU5WjCB
qTELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1Nb3VudGFpbiBW
aWV3MRwwGgYDVQQKExNBZGRvbnMgVGVzdCBTaWduaW5nMSQwIgYDVQQDExt0ZXN0
LmFkZG9ucy5zaWduaW5nLnJvb3QuY2ExMTAvBgkqhkiG9w0BCQEWInNlY29wcytz
dGFnZXJvb3RhZGRvbnNAbW96aWxsYS5jb20wggIiMA0GCSqGSIb3DQEBAQUAA4IC
DwAwggIKAoICAQDKRVty/FRsO4Ech6EYleyaKgAueaLYfMSsAIyPC/N8n/P8QcH8
rjoiMJrKHRlqiJmMBSmjUZVzZAP0XJku0orLKWPKq7cATt+xhGY/RJtOzenMMsr5
eN02V3GzUd1jOShUpERjzXdaO3pnfZqhdqNYqP9ocqQpyno7bZ3FZQ2vei+bF52k
51uPioTZo+1zduoR/rT01twGtZm3QpcwU4mO74ysyxxgqEy3kpojq8Nt6haDwzrj
khV9M6DGPLHZD71QaUiz5lOhD9CS8x0uqXhBhwMUBBkHsUDSxbN4ZhjDDWpCmwaD
OtbJMUJxDGPCr9qj49QESccb367OeXLrfZ2Ntu/US2Bw9EDfhyNsXr9dg9NHj5yf
4sDUqBHG0W8zaUvJx5T2Ivwtno1YZLyJwQW5pWeWn8bEmpQKD2KS/3y2UjlDg+YM
NdNASjFe0fh6I5NCFYmFWA73DpDGlUx0BtQQU/eZQJ+oLOTLzp8d3dvenTBVnKF+
uwEmoNfZwc4TTWJOhLgwxA4uK+Paaqo4Ap2RGS2ZmVkPxmroB3gL5n3k3QEXvULh
7v8Psk4+MuNWnxudrPkN38MGJo7ju7gDOO8h1jLD4tdfuAqbtQLduLXzT4DJPA4y
JBTFIRMIpMqP9CovaS8VPtMFLTrYlFh9UnEGpCeLPanJr+VEj7ae5sc8YwIDAQAB
o4IBmDCCAZQwDAYDVR0TBAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwFgYDVR0lAQH/
BAwwCgYIKwYBBQUHAwMwLAYJYIZIAYb4QgENBB8WHU9wZW5TU0wgR2VuZXJhdGVk
IENlcnRpZmljYXRlMDMGCWCGSAGG+EIBBAQmFiRodHRwOi8vYWRkb25zLm1vemls
bGEub3JnL2NhL2NybC5wZW0wHQYDVR0OBBYEFIbYNBxOWNETXJlf2EKY7RQPGfJd
MIHZBgNVHSMEgdEwgc6AFIbYNBxOWNETXJlf2EKY7RQPGfJdoYGvpIGsMIGpMQsw
CQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExFjAUBgNVBAcTDU1vdW50YWluIFZpZXcx
HDAaBgNVBAoTE0FkZG9ucyBUZXN0IFNpZ25pbmcxJDAiBgNVBAMTG3Rlc3QuYWRk
b25zLnNpZ25pbmcucm9vdC5jYTExMC8GCSqGSIb3DQEJARYic2Vjb3BzK3N0YWdl
cm9vdGFkZG9uc0Btb3ppbGxhLmNvbYIEYCWYOzANBgkqhkiG9w0BAQwFAAOCAgEA
nowyJv8UaIV7NA0B3wkWratq6FgA1s/PzetG/ZKZDIW5YtfUvvyy72HDAwgKbtap
Eog6zGI4L86K0UGUAC32fBjE5lWYEgsxNM5VWlQjbgTG0dc3dYiufxfDFeMbAPmD
DzpIgN3jHW2uRqa/MJ+egHhv7kGFL68uVLboqk/qHr+SOCc1LNeSMCuQqvHwwM0+
AU1GxhzBWDkealTS34FpVxF4sT5sKLODdIS5HXJr2COHHfYkw2SW/Sfpt6fsOwaF
2iiDaK4LPWHWhhIYa6yaynJ+6O6KPlpvKYCChaTOVdc+ikyeiSO6AakJykr5Gy7d
PkkK7MDCxuY6psHj7iJQ59YK7ujQB8QYdzuXBuLLo5hc5gBcq3PJs0fLT2YFcQHA
dj+olGaDn38T0WI8ycWaFhQfKwATeLWfiQepr8JfoNlC2vvSDzGUGfdAfZfsJJZ8
5xZxahHoTFGS0mDRfXqzKH5uD578GgjOZp0fULmzkcjWsgzdpDhadGjExRZFKlAy
iKv8cXTONrGY0fyBDKennuX0uAca3V0Qm6v2VRp+7wG/pywWwc5n+04qgxTQPxgO
6pPB9UUsNbaLMDR5QPYAWrNhqJ7B07XqIYJZSwGP5xB9NqUZLF4z+AOMYgWtDpmg
IKdcFKAt3fFrpyMhlfIKkLfmm0iDjmfmIXbDGBJw9SE=
-----END CERTIFICATE-----";
    const VALID_INPUT: &[u8] =
        b"Content-Signature:\x00{\"data\":[],\"last_modified\":\"1603992731957\"}";
    const VALID_SIGNATURE: &str = "fJJcOpwdnkjEWFeHXfdOJN6GaGLuDTPGzQOxA2jn6ldIleIk6KqMhZcy2GZv2uYiGwl6DERWwpaoUfQFLyCAOcVjck1qlaaEFZGY1BQba9p99xEc9FNQ3YPPfvSSZqsw";
    const VALID_HOSTNAME: &str = "remote-settings.content-signature.mozilla.org";

    const INVALID_CERTIFICATE: &[u8] = b"\
    -----BEGIN CERTIFICATE-----
    invalidCertificategIFiJLFfdxFlYwCgYIKoZIzj0EAwMwgaMxCzAJBgNVBAYT
    AlVTMRwwGgYDVQQKExNNb3ppbGxhIENvcnBvcmF0aW9uMS8wLQYDVQQLEyZNb3pp
    bGxhIEFNTyBQcm9kdWN0aW9uIFNpZ25pbmcgU2VydmljZTFFMEMGA1UEAww8Q29u
    dGVudCBTaWduaW5nIEludGVybWVkaWF0ZS9lbWFpbEFkZHJlc3M9Zm94c2VjQG1v
    emlsbGEuY29tMB4XDTIwMDYxNjE3MTYxNVoXDTIwMDkwNDE3MTYxNVowgakxCzAJ
    BgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQHEw1Nb3VudGFp
    biBWaWV3MRwwGgYDVQQKExNNb3ppbGxhIENvcnBvcmF0aW9uMRcwFQYDVQQLEw5D
    bG91ZCBTZXJ2aWNlczE2MDQGA1UEAxMtcmVtb3RlLXNldHRpbmdzLmNvbnRlbnQt
    c2lnbmF0dXJlLm1vemlsbGEub3JnMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEDmOX
    N5IGlUqCvu6xkOKr020Eo3kY2uPdJO0ZihVUoglk1ktQPss184OajFOMKm/BJX4W
    IsZUzQoRL8NgGfZDwBjT95Q87lhOWEWs5AU/nMXIYwDp7rpUPaUqw0QLMikdo4GD
    MIGAMA4GA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAfBgNVHSME
    GDAWgBSgHUoXT4zCKzVF8WPx2nBwp8744TA4BgNVHREEMTAvgi1yZW1vdGUtc2V0
    dGluZ3MuY29udGVudC1zaWduYXR1cmUubW96aWxsYS5vcmcwCgYIKoZIzj0EAwMD
    aQAwZgIxAJvyynyPqRmRMqf95FPH5xfcoT3jb/2LOkUifGDtjtZ338ScpT2glUK8
    HszKVANqXQIxAIygMaeTiD9figEusmHMthBdFoIoHk31x4MHukAy+TWZ863X6/V2
    6/ZrZMpinvalid==
    -----END CERTIFICATE-----";

    // Fetched from: https://content-signature-2.cdn.mozilla.net/chains/aus.content-signature.mozilla.org-2023-11-18-16-07-40.chain
    const PROD_CERT_CHAIN: &[u8] = b"\
-----BEGIN CERTIFICATE-----
MIIC7DCCAnGgAwIBAgIIF4lqV840D50wCgYIKoZIzj0EAwMwgaMxCzAJBgNVBAYT
AlVTMRwwGgYDVQQKExNNb3ppbGxhIENvcnBvcmF0aW9uMS8wLQYDVQQLEyZNb3pp
bGxhIEFNTyBQcm9kdWN0aW9uIFNpZ25pbmcgU2VydmljZTFFMEMGA1UEAww8Q29u
dGVudCBTaWduaW5nIEludGVybWVkaWF0ZS9lbWFpbEFkZHJlc3M9Zm94c2VjQG1v
emlsbGEuY29tMB4XDTIzMDgzMDE2MDc0MFoXDTIzMTExODE2MDc0MFowgZ0xCzAJ
BgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQHEw1Nb3VudGFp
biBWaWV3MRwwGgYDVQQKExNNb3ppbGxhIENvcnBvcmF0aW9uMRcwFQYDVQQLEw5D
bG91ZCBTZXJ2aWNlczEqMCgGA1UEAxMhYXVzLmNvbnRlbnQtc2lnbmF0dXJlLm1v
emlsbGEub3JnMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEnZd6AgUxhS7BLlh+jlO6
kqEJz6gRXPnv6QGLwxZU0IWT5oqGnR+A667g32U0bMjJTEskyX7IGZ5iCtlMV3Ik
TT5OujCQwhN9fDlhqepmmjbumXNcUMtFw/QTa0yJc+fFo3YwdDAOBgNVHQ8BAf8E
BAMCB4AwEwYDVR0lBAwwCgYIKwYBBQUHAwMwHwYDVR0jBBgwFoAUoB1KF0+Mwis1
RfFj8dpwcKfO+OEwLAYDVR0RBCUwI4IhYXVzLmNvbnRlbnQtc2lnbmF0dXJlLm1v
emlsbGEub3JnMAoGCCqGSM49BAMDA2kAMGYCMQDcjyz+FvKgOMGJqXnJBJ8fD9YV
mLmZDPng6Wr9hyGBhsJyelGR1tzPYXZWXUPsbAsCMQCtbgzFRKq5KKViqto1iFK+
LZPYG2qEJ0x5I2vLD/LmoR6pkWqz5kqeZ6k6CwDm+tg=
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIFfzCCA2egAwIBAgIDEAAJMA0GCSqGSIb3DQEBDAUAMH0xCzAJBgNVBAYTAlVT
MRwwGgYDVQQKExNNb3ppbGxhIENvcnBvcmF0aW9uMS8wLQYDVQQLEyZNb3ppbGxh
IEFNTyBQcm9kdWN0aW9uIFNpZ25pbmcgU2VydmljZTEfMB0GA1UEAxMWcm9vdC1j
YS1wcm9kdWN0aW9uLWFtbzAiGA8yMDIwMTIzMTAwMDAwMFoYDzIwMjUwMzE0MjI1
MzU3WjCBozELMAkGA1UEBhMCVVMxHDAaBgNVBAoTE01vemlsbGEgQ29ycG9yYXRp
b24xLzAtBgNVBAsTJk1vemlsbGEgQU1PIFByb2R1Y3Rpb24gU2lnbmluZyBTZXJ2
aWNlMUUwQwYDVQQDDDxDb250ZW50IFNpZ25pbmcgSW50ZXJtZWRpYXRlL2VtYWls
QWRkcmVzcz1mb3hzZWNAbW96aWxsYS5jb20wdjAQBgcqhkjOPQIBBgUrgQQAIgNi
AAQklaclq89BKPcYIfUdVS4JF/pZxjTVOlYVdj4QJ5xopHAngXkUggYkkHj0tmZV
EcrXrCVq1qEtB/k7wXXkU4HN9rX7WcUkksClDJmUQ2qabzP7i20q3epGNq57RE2p
3hKjggGJMIIBhTAMBgNVHRMEBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAWBgNVHSUB
Af8EDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUoB1KF0+Mwis1RfFj8dpwcKfO+OEw
gagGA1UdIwSBoDCBnYAUs7zqWHSr4W54KrKrnCMeqGMsl7ehgYGkfzB9MQswCQYD
VQQGEwJVUzEcMBoGA1UEChMTTW96aWxsYSBDb3Jwb3JhdGlvbjEvMC0GA1UECxMm
TW96aWxsYSBBTU8gUHJvZHVjdGlvbiBTaWduaW5nIFNlcnZpY2UxHzAdBgNVBAMT
FnJvb3QtY2EtcHJvZHVjdGlvbi1hbW+CAQEwMwYJYIZIAYb4QgEEBCYWJGh0dHA6
Ly9hZGRvbnMuYWxsaXpvbS5vcmcvY2EvY3JsLnBlbTBOBgNVHR4ERzBFoEMwIIIe
LmNvbnRlbnQtc2lnbmF0dXJlLm1vemlsbGEub3JnMB+CHWNvbnRlbnQtc2lnbmF0
dXJlLm1vemlsbGEub3JnMA0GCSqGSIb3DQEBDAUAA4ICAQALeUuF/7hcmM/LFnK6
6a5lBQk5z5JBr2bNNvKVs/mtdIcVKcxjWxOBM5rorZiM5UWE7BmAm8E7gFCCq30y
ZnNn6BO04z5LtDRHxa3IGhgECloyOJUSi9xxFxe5p5wJzFdArl7happSOUwOi+z2
aDqS6uTJWubIY4Uz7h7S2UkUm52CTYnvpioS7eQoovvrlUsgIhkkIwDQnu7RWSej
6nkc5o5SNwAJWsQvxIko32AxhvPmmtv1T/mtXY488TJ0VoBZ6lRkJJIxIJ48pGHJ
+YRt1tzO2aqCEs9pNPGWfhrpcDc2mu4fvlSX1elWYiGrpQBVbdEJlDkGAD0AC8on
/7ybD2pEdh7pViVLV78Md+DNNquqqNhRJpn65k4lhvgDLHYvLNOrrtAmcmQonNdU
OSumIuqcGk7dm/7gr9lrwAm8V8/GwDyzTgi4wNA4vwln3c7iMFGLL/b2piEQCSl+
mqL1LeWJV+8rkbi8l2T0QIBwjDgR97ZxpLPwmUdDNiGAWeEFxn0jU9CQtQKjOj84
VPZUM7aSHhVQ0bQpnjua7IWvLKK7F2fOo3PmuLacnnfyrzr2C/Le5k6EK/0q2cKf
P6JzDWwt8werc6E3C6z3jbUdAwgNpv/fGz8gQBPf7NeiYkqMUNB2Z3aF8He8Jg15
Abv+2+rSOLpBsTU67AHzMKJ8hw==
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIGYTCCBEmgAwIBAgIBATANBgkqhkiG9w0BAQwFADB9MQswCQYDVQQGEwJVUzEc
MBoGA1UEChMTTW96aWxsYSBDb3Jwb3JhdGlvbjEvMC0GA1UECxMmTW96aWxsYSBB
TU8gUHJvZHVjdGlvbiBTaWduaW5nIFNlcnZpY2UxHzAdBgNVBAMTFnJvb3QtY2Et
cHJvZHVjdGlvbi1hbW8wHhcNMTUwMzE3MjI1MzU3WhcNMjUwMzE0MjI1MzU3WjB9
MQswCQYDVQQGEwJVUzEcMBoGA1UEChMTTW96aWxsYSBDb3Jwb3JhdGlvbjEvMC0G
A1UECxMmTW96aWxsYSBBTU8gUHJvZHVjdGlvbiBTaWduaW5nIFNlcnZpY2UxHzAd
BgNVBAMTFnJvb3QtY2EtcHJvZHVjdGlvbi1hbW8wggIgMA0GCSqGSIb3DQEBAQUA
A4ICDQAwggIIAoICAQC0u2HXXbrwy36+MPeKf5jgoASMfMNz7mJWBecJgvlTf4hH
JbLzMPsIUauzI9GEpLfHdZ6wzSyFOb4AM+D1mxAWhuZJ3MDAJOf3B1Rs6QorHrl8
qqlNtPGqepnpNJcLo7JsSqqE3NUm72MgqIHRgTRsqUs+7LIPGe7262U+N/T0LPYV
Le4rZ2RDHoaZhYY7a9+49mHOI/g2YFB+9yZjE+XdplT2kBgA4P8db7i7I0tIi4b0
B0N6y9MhL+CRZJyxdFe2wBykJX14LsheKsM1azHjZO56SKNrW8VAJTLkpRxCmsiT
r08fnPyDKmaeZ0BtsugicdipcZpXriIGmsZbI12q5yuwjSELdkDV6Uajo2n+2ws5
uXrP342X71WiWhC/dF5dz1LKtjBdmUkxaQMOP/uhtXEKBrZo1ounDRQx1j7+SkQ4
BEwjB3SEtr7XDWGOcOIkoJZWPACfBLC3PJCBWjTAyBlud0C5n3Cy9regAAnOIqI1
t16GU2laRh7elJ7gPRNgQgwLXeZcFxw6wvyiEcmCjOEQ6PM8UQjthOsKlszMhlKw
vjyOGDoztkqSBy/v+Asx7OW2Q7rlVfKarL0mREZdSMfoy3zTgtMVCM0vhNl6zcvf
5HNNopoEdg5yuXo2chZ1p1J+q86b0G5yJRMeT2+iOVY2EQ37tHrqUURncCy4uwIB
A6OB7TCB6jAMBgNVHRMEBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAWBgNVHSUBAf8E
DDAKBggrBgEFBQcDAzCBkgYDVR0jBIGKMIGHoYGBpH8wfTELMAkGA1UEBhMCVVMx
HDAaBgNVBAoTE01vemlsbGEgQ29ycG9yYXRpb24xLzAtBgNVBAsTJk1vemlsbGEg
QU1PIFByb2R1Y3Rpb24gU2lnbmluZyBTZXJ2aWNlMR8wHQYDVQQDExZyb290LWNh
LXByb2R1Y3Rpb24tYW1vggEBMB0GA1UdDgQWBBSzvOpYdKvhbngqsqucIx6oYyyX
tzANBgkqhkiG9w0BAQwFAAOCAgEAaNSRYAaECAePQFyfk12kl8UPLh8hBNidP2H6
KT6O0vCVBjxmMrwr8Aqz6NL+TgdPmGRPDDLPDpDJTdWzdj7khAjxqWYhutACTew5
eWEaAzyErbKQl+duKvtThhV2p6F6YHJ2vutu4KIciOMKB8dslIqIQr90IX2Usljq
8Ttdyf+GhUmazqLtoB0GOuESEqT4unX6X7vSGu1oLV20t7t5eCnMMYD67ZBn0YIU
/cm/+pan66hHrja+NeDGF8wabJxdqKItCS3p3GN1zUGuJKrLykxqbOp/21byAGog
Z1amhz6NHUcfE6jki7sM7LHjPostU5ZWs3PEfVVgha9fZUhOrIDsyXEpCWVa3481
LlAq3GiUMKZ5DVRh9/Nvm4NwrTfB3QkQQJCwfXvO9pwnPKtISYkZUqhEqvXk5nBg
QCkDSLDjXTx39naBBGIVIqBtKKuVTla9enngdq692xX/CgO6QJVrwpqdGjebj5P8
5fNZPABzTezG3Uls5Vp+4iIWVAEDkK23cUj3c/HhE+Oo7kxfUeu5Y1ZV3qr61+6t
ZARKjbu1TuYQHf0fs+GwID8zeLc2zJL7UzcHFwwQ6Nda9OJN4uPAuC/BKaIpxCLL
26b24/tRam4SJjqpiq20lynhUrmTtt6hbG3E1Hpy3bmkt2DYnuMFwEx2gfXNcnbT
wNuvFqc=
-----END CERTIFICATE-----";
    const PROD_ROOT_HASH: &str = "97e8ba9cf12fb3de53cc42a4e6577ed64df493c247b414fea036818d3823560e";
    const PROD_HOSTNAME: &str = "aus.content-signature.mozilla.org";
    // Fetched from: https://aus5.mozilla.org/json/1/FirefoxVPN/2.14.0/WINNT_x86_64/release-cdntest/update.json
    const PROD_SIGNATURE: &str = "znYFqdKKFgijVgUhnq5VuZxtI5Zay8MARVFr3cG1CbB9eH9slQFkE9ZjMdLzbf5OZqj2gds1OqbCm45L38e2joKD_mCAUGtajebztDdWx9Rqgmn-9vu6t-SCl6HQrzbh";
    const PROD_INPUT_DATA: &[u8] =
        b"Content-Signature:\x00{\"version\": \"2.17.0\", \"url\": \"https://archive.mozilla.org/pub/vpn/candidates/2.17.0-candidates/build20230926170708/windows/MozillaVPN.msi\", \"required\": false, \"hashFunction\": \"sha512\", \"hashValue\": \"97fe0255e50cd33d3ed50fe94edf3d95b72950b28f9b0157b43195a6c190aaf7e34543044c14d42deabf97b84910da1fbb7f752eaf800130315c2ad5172ea45c\"}";

    #[test]
    fn test_verify_succeeds_if_valid() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            VALID_SIGNATURE,
            1615559719, // March 12, 2021
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
            1615559719, // March 12, 2021
            &ROOT_HASH.replace("A", "B"),
            VALID_HOSTNAME,
        );
        assert_eq!(r, Err(BalrogError::RootHashMismatch));
    }

    #[test]
    fn test_verify_fails_if_cert_invalid() {
        let r = parse_and_verify(
            INVALID_CERTIFICATE,
            VALID_INPUT,
            VALID_SIGNATURE,
            1615559719, // March 12, 2021
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        /* I think this is the wrong error code actually...  */
        /* I would have expected X509Error::InvalidCertificate */
        assert_eq!(r, Err(BalrogError::PemDecodeError));
    }

    #[test]
    fn test_verify_fails_if_hostname_mismatch() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            VALID_SIGNATURE,
            1615559719, // March 12, 2021
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
    fn test_verify_fails_if_bad_siganture() {
        let r = parse_and_verify(
            VALID_CERT_CHAIN,
            VALID_INPUT,
            &VALID_SIGNATURE.to_ascii_lowercase(),
            1615559719, // March 12, 2021
            ROOT_HASH,
            VALID_HOSTNAME,
        );
        assert_eq!(r, Err(BalrogError::from(X509Error::SignatureVerificationError)));
    }
    #[test]
    fn test_verify_prod_example() {
        let r = parse_and_verify(
            PROD_CERT_CHAIN,
            PROD_INPUT_DATA,
            PROD_SIGNATURE,
            1696272799, // Oct 2, 2023 - the time when I wrote this test
            PROD_ROOT_HASH,
            PROD_HOSTNAME
        );
        assert!(r.is_ok(), "Found unexpected error: {}", r.unwrap_err());
    }

    /* TODO: We could use tests for:
     *  - Missing code signing permissions.
     *  - A certificate chain that doesn't chain.
     *  - Chains of irregular length.
     *  - Other signature algorithms.
     *  - A PEM file that contains something other than a certificate.
     */
}
