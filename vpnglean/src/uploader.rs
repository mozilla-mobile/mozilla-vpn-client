use glean::net::{PingUploader, UploadResult};
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

// TODO: Write this properly.
impl PingUploader for VPNPingUploader {
    fn upload(&self, url: String, body: Vec<u8>, headers: Vec<(String, String)>) -> UploadResult {
        let mut parsed_headers = HeaderMap::new();
        for (name, value) in headers {
            parsed_headers.insert(
                HeaderName::from_bytes(&name.as_bytes()).unwrap(),
                HeaderValue::from_str(&value).unwrap(),
            );
        }

        let response = self
            .client
            .post(url)
            .headers(parsed_headers)
            .body(body)
            .send();

        match response {
            Ok(result) => {
                UploadResult::http_status((result.status().as_u16() as u32).try_into().unwrap())
            }
            Err(_) => UploadResult::unrecoverable_failure(),
        }
    }
}
