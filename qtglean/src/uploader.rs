/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use glean::net::{PingUploader, UploadResult, PingUploadRequest, CapablePingUploadRequest};
use std::ffi::CString;
use std::ffi::NulError;
use std::os::raw::c_char;
use std::os::raw::c_uchar;
use std::os::raw::c_int;
use std::sync::Mutex;
use std::sync::Arc;
use ::core::slice;

#[repr(C)]
#[derive(Clone)]
pub struct VPNPingHeader {
    name: *const c_char,
    value: *const c_char,
}

#[repr(C)]
pub struct VPNPingPayload {
    url: *const c_char,
    body: *const c_uchar,
    length: usize,
    has_info: bool,
    header_list: *const VPNPingHeader,
    header_count: usize,
}

impl VPNPingPayload {
    pub fn new(pingreq: &PingUploadRequest) -> Result<Self, NulError> {
        let url = CString::new(pingreq.url.as_bytes())?;

        let mut cstr_headers: Vec<(CString,CString)> = Vec::with_capacity(pingreq.headers.len());
        for (name, value) in &pingreq.headers {
            let cstr_name = CString::new(name.as_str())?;
            let cstr_value = CString::new(value.as_str())?;
            cstr_headers.push((cstr_name, cstr_value));
        }
        let mut ffi_headers: Vec<VPNPingHeader> = Vec::with_capacity(pingreq.headers.len());
        for (name, value) in cstr_headers {
            ffi_headers.push(VPNPingHeader{
                name: name.into_raw(),
                value: value.into_raw(),
            });
        }
        let ffi_slice = Box::into_raw(ffi_headers.into_boxed_slice());

        Ok(VPNPingPayload{
            url: url.into_raw(),
            body: pingreq.body.as_ptr(),
            length: pingreq.body.len(),
            has_info: pingreq.body_has_info_sections,
            header_list: ffi_slice as *const VPNPingHeader,
            header_count: ffi_slice.len(),
        })
    }
}

impl Drop for VPNPingPayload {
    fn drop(&mut self) {
        unsafe {
            let _url = CString::from_raw(self.url as *mut c_char);
            let ffi_slice = slice::from_raw_parts_mut(self.header_list as *mut VPNPingHeader, self.header_count);
            let ffi_boxed = Box::from_raw(ffi_slice);
            for x in ffi_boxed {
                let _n = CString::from_raw(x.name as *mut c_char);
                let _v = CString::from_raw(x.value as *mut c_char);
            }
        };
    }
}

#[derive(Debug)]
pub struct VPNPingUploader {
    forbidden_ping_list: Arc<Mutex<Vec<String>>>,
    handler: extern "C" fn(*const VPNPingPayload) -> c_int,
}

impl VPNPingUploader {
    pub fn new(handler: extern "C" fn(*const VPNPingPayload) -> c_int,
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
}

impl PingUploader for VPNPingUploader {
    fn upload(&self, upload_request: CapablePingUploadRequest) -> UploadResult {
        let upload_request = upload_request.capable(|cap| cap.is_empty()).unwrap();
        if !self.allowed_to_send(&upload_request){
            // If we're not allowed to send, "fake" a 200 response, 
            // so the data is dropped and not retried later. 
            return UploadResult::http_status(200); 
        }

        let Ok(payload) = VPNPingPayload::new(&upload_request) else {
            return UploadResult::unrecoverable_failure();
        };
        let callback = self.handler;
        let response = callback(&payload);

        if response < 0 {
            UploadResult::unrecoverable_failure()
        } else if response == 0 {
            UploadResult::done()
        } else {
            UploadResult::http_status(response)
        }
    }
}
