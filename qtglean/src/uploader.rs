/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use glean::net::{PingUploader, UploadResult, PingUploadRequest};
use reqwest::blocking::Client;
use reqwest::header::{HeaderMap, HeaderName, HeaderValue};

#[derive(Debug)]
pub struct VPNPingUploader {
    client: Client,
}

impl VPNPingUploader {
    pub fn new() -> VPNPingUploader {
        VPNPingUploader {
            client: Client::new(),
        }
    }
}

impl PingUploader for VPNPingUploader {
    fn upload(&self, upload_request: PingUploadRequest) -> UploadResult {
        let mut parsed_headers = HeaderMap::new();
        for (name, value) in upload_request.headers {
            if let (Ok(parsed_name), Ok(parsed_value)) = (
                HeaderName::from_bytes(&name.as_bytes()),
                HeaderValue::from_str(&value),
            ) {
                parsed_headers.insert(parsed_name, parsed_value);
            }
        }

        let response = self
            .client
            .post(upload_request.url)
            .headers(parsed_headers)
            .body(upload_request.body)
            .send();

        let status = match response {
            Ok(result) => Some(result.status()),
            Err(error) => error.status(),
        };

        if status.is_none() {
            // If we didn't even get a status this is likely a network issue i.e. recoverable.
            return UploadResult::recoverable_failure();
        }

        if let Ok(parsed_status) = i32::try_from(status.unwrap().as_u16()) {
            return UploadResult::http_status(parsed_status);
        };

        UploadResult::unrecoverable_failure()
    }
}
