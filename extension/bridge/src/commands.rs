/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */



 use std::error::Error;
 use serde_json::{Value, json};
 
 /**
  * Handles commands that are sent from
  * [Extension] === > [NativeMessagingBridge]
  * 
  * Returns true if the command was handled, in which case it should
  * *not* be forwarded to the VPN Client. 
  * 
  * Will attempt to print to STDOUT in case a command needs a response. 
  *  
  */
 pub fn handle(val:&Value)-> Result<bool,Box<dyn Error>>{
     let obj = val.as_object().ok_or("Not an object")?;
     // Type of command is in {t:'doThing'}
     let cmd = obj.get_key_value("t").ok_or("Missing obj.t")?;
 
     match cmd.1.as_str().ok_or("T is not a string")? {
         "bridge_pong" =>{
             crate::io::write_output(std::io::stdout(),&json!({"status": "bridge_pong"}))
                 .expect("Unable to Write to STDOUT?");
             return Ok(true);
         }
         "start" =>{
            let out = launcher::start_vpn();
            crate::io::write_output(std::io::stdout(),&out)
                .expect("Unable to Write to STDOUT?");
            return Ok(true);
        }
         _ =>{
             // We did not handle this.
             return Ok(false);
         }
     }
 }
 

#[cfg(target_os = "windows")]
mod launcher {
    const CLIENT_PATH: &str = "C:\\Program Files\\Mozilla\\Mozilla VPN\\Mozilla VPN.exe";

    use std::os::windows::process::CommandExt;
    use std::process::Command;

    use serde_json::json;

    const CREATE_NEW_PROCESS_GROUP: u32 = 0x200; // CREATE_NEW_PROCESS_GROUP
    const DETACHED_PROCESS: u32 = 0x00000008;    // DETACHED_PROCESS

    pub fn start_vpn() -> serde_json::Value{
        let result = Command::new(CLIENT_PATH)
            .args(["-foreground"])
            .creation_flags(CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS)
            .spawn();

        match result {
            Ok(_) => json!("{status:'requested_start'}"),
            Err(_) => json!("{error:'start_failed'}"),
        }
    }


}

#[cfg(not(target_os = "windows"))]
mod launcher {
    pub fn start_vpn() -> serde_json::Value{
        json!("{error:'start_unsupported!'}")
    }
}
