/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */



 use std::error::Error;
 use serde_json::{Value, json};
 use sysinfo;

 
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
         "bridge_ping" =>{
             crate::io::write_output(std::io::stdout(),&json!({"status": "bridge_pong"}))
                 .expect("Unable to Write to STDOUT?");
             Ok(true)
         }
         "start" =>{
            let out = launcher::start_vpn();
            crate::io::write_output(std::io::stdout(),&out)
                .expect("Unable to Write to STDOUT?");
            Ok(true)
        }
        "proc_info" => {
            let parent_pid = get_parent_pid();
            let s = sysinfo::System::new_all();

            if let Some(parent_process) = s.process(sysinfo::Pid::from_u32(parent_pid)) {
                let parent_info = json!({
                    "b":"proc_info",
                    "pid": parent_process.pid().as_u32(),
                    "exe": parent_process.exe()
                });
                crate::io::write_output(std::io::stdout(), &parent_info)
                    .expect("Unable to Write to STDOUT?");
            } else {
                crate::io::write_output(
                    std::io::stdout(),
                    &json!({"error": "Parent process not found"}),
                )
                .expect("Unable to Write to STDOUT?");
            }

            Ok(true)
        }
         _ =>{
             // We did not handle this.
             Ok(false)
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
    use serde_json::json;
    pub fn start_vpn() -> serde_json::Value{
        json!("{error:'start_unsupported!'}")
    }
}



#[cfg(target_os = "windows")]
fn get_parent_pid() -> u32 {
    use std::mem;
    use windows::Win32::Foundation::{CloseHandle, HANDLE};
    use windows::Win32::System::Diagnostics::ToolHelp::{
        CreateToolhelp32Snapshot, Process32First, Process32Next, PROCESSENTRY32,
        TH32CS_SNAPPROCESS,
    };

    let mut ppid: u32 = 0;
    let pid = std::process::id();

    unsafe {
        let snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
            .expect("Failed to create snapshot");
        let snapshot_handle = HANDLE(snapshot.0);

        let mut entry = PROCESSENTRY32 {
            dwSize: mem::size_of::<PROCESSENTRY32>() as u32,
            ..Default::default()
        };

        if Process32First(snapshot_handle, &mut entry).is_ok() {
            while Process32Next(snapshot_handle, &mut entry).is_ok() {
                if entry.th32ProcessID == pid {
                    ppid = entry.th32ParentProcessID;
                    break;
                }
            }
        }

        let _ = CloseHandle(snapshot_handle);
    }
    ppid
}

#[cfg(any(target_os = "macos", target_os = "linux"))]
fn get_parent_pid() -> u32 {
    return os::unix::process::parent_id;
}
