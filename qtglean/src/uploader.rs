/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use glean::net::{PingUploader, UploadResult, PingUploadRequest, CapablePingUploadRequest};
use std::ffi::CString;
use std::os::raw::c_char;
use std::os::raw::c_uchar;
use std::os::raw::c_int;
use std::ptr;
use std::sync::Mutex;
use std::sync::Arc;

#[repr(C)]
pub struct vpn_ping_header {
    name: *const c_char,
    value: *const c_char,
}

#[repr(C)]
pub struct vpn_ping_payload {
    url: *const c_char,
    body: *const c_uchar,
    length: usize,
    has_info: bool,
    headers: *const vpn_ping_header,
}

#[derive(Debug)]
pub struct VPNPingUploader {
    forbidden_ping_list: Arc<Mutex<Vec<String>>>,
    handler: extern "C" fn(*const vpn_ping_payload) -> c_int,
}

impl VPNPingUploader {
    pub fn new(handler: extern "C" fn(*const vpn_ping_payload) -> c_int,
               forbidden_pings: Arc<Mutex<Vec<String>>>) -> VPNPingUploader {
        VPNPingUploader {
            forbidden_ping_list: forbidden_pings,
            handler: handler,
        }
    }

    fn allowed_to_send(&self, req: &PingUploadRequest)->bool{
        let data = self.forbidden_ping_list.lock().unwrap();
        // Emtpy list -> allow everything 
        if data.is_empty(){
            return true; 
        }
        // Otherwise the ping_name MUST NOT be in the list.
        return data.iter().all(|e| e != &req.ping_name);
    }

    fn free_ffi_headers(&self, headers: Vec<vpn_ping_header>) {
        unsafe {
            for x in headers {
                if !x.name.is_null() {
                    let _ = CString::from_raw(x.name as *mut c_char);
                }
                if !x.value.is_null() {
                    let _ = CString::from_raw(x.value as *mut c_char);
                }
            }
        }
    }
}

impl PingUploader for VPNPingUploader {
    fn upload(&self, upload_request: CapablePingUploadRequest) -> UploadResult {
        let upload_request = upload_request.capable(|cap| cap.is_empty()).unwrap();
        if !self.allowed_to_send(&upload_request){
            // If we're not allowed to send, "fake" a 200 response, 
            // so the data is dropped and not retried later. 
            return UploadResult::http_status(200); 
        }
        let Ok(url) = CString::new(upload_request.url) else {
            return UploadResult::unrecoverable_failure();
        };

        // Garbage collect the raw CStrings
        let mut ffiHeaders: Vec<vpn_ping_header> = Vec::new();
        ffiHeaders.reserve(upload_request.headers.len() + 1);
        for (name, value) in upload_request.headers {
            let Ok(cName) = CString::new(name) else {
                self.free_ffi_headers(ffiHeaders);
                return UploadResult::unrecoverable_failure();
            };
            let Ok(cValue) = CString::new(value) else {
                self.free_ffi_headers(ffiHeaders);
                return UploadResult::unrecoverable_failure();
            };
            ffiHeaders.push(vpn_ping_header{
                name: cName.into_raw(),
                value: cValue.into_raw(),
            });
        }
        ffiHeaders.push(vpn_ping_header{
            name: ptr::null(),
            value: ptr::null(),
        });

        let payload = vpn_ping_payload{
            url: url.as_ptr(),
            body: upload_request.body.as_ptr(),
            length: upload_request.body.len(),
            has_info: upload_request.body_has_info_sections,
            headers: ffiHeaders.as_ptr(),
        };
        let callback = self.handler;
        let response = callback(&payload);
        self.free_ffi_headers(ffiHeaders);

        if response < 0 {
            UploadResult::unrecoverable_failure()
        } else if response == 0 {
            UploadResult::done()
        } else {
            UploadResult::http_status(response)
        }
    }
}
