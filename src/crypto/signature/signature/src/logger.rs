use std::ffi::CString;
use std::os::raw::c_char;

pub struct SignatureLogger {
    pub handler: Option<extern "C" fn(*const c_char)>,
}

impl SignatureLogger {
    pub fn print(&self, msg: &str) {
        let callback = match self.handler {
            None => {
                eprintln!("{}", msg);
                return;
            }
            Some(x) => x,
        };

        let _ = match CString::new(msg) {
            Err(_e) => {}
            Ok(x) => unsafe {
                let msg_cstr = x.into_raw();
                callback(msg_cstr);
                let _ = CString::from_raw(msg_cstr);
            },
        };
    }
}
