/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use byteorder::{NativeEndian, ReadBytesExt, WriteBytesExt};
use mio::net::TcpStream;
use mio::{Events, Interest, Poll, Token, Waker};
use serde_json::{json, Value};
use std::io::{Cursor, Read, Write};
use std::mem::size_of;
use std::sync::mpsc::channel;
use std::sync::Arc;
use std::{thread, time};
use std::env;

mod commands;
mod io;

const SERVER_AND_PORT: &str = "127.0.0.1:8754";

const ALLOW_LISTED_WEBEXTENSIONS: [&str;2] = [
    "@testpilot-containers",
    "vpn@mozilla.com"
];


fn main() {
    /*
        There are two arguments are passed to this binary when it starts.
        - The complete path to the app manifest.
        - The ID (as given in the browser_specific_settings manifest.json key) of the add-on that started it.

        Note: Chrome handles the passed arguments differently - we need to change that, if we ever want to support that. 
    */
    let arguments: Vec<String> = env::args().collect();
    if arguments.len() != 3 { 
        println!("Expected 2 arguments got: {}", arguments.len() -1 );
        println!("Please invoke using <manifest path> <extension id> ");
        std::process::exit(1);
    }
    let ext_id = arguments.last().expect("We cannot start without an extension id");
    if !ALLOW_LISTED_WEBEXTENSIONS.contains(&ext_id.as_str()){
        println!("mozillavpnnp is not accessible for extension: {}", ext_id.as_str());
        std::process::exit(1);
    }


    // A poll to process VPN and STDIN events.
    let mut poll = Poll::new().expect("Failed to allocate a poll");
    let mut events = Events::with_capacity(128);

    // A channel to connect the STDIN-reader thread with the main one.
    let (sender, receiver) = channel();

    // Two tokens for VPN to IO and IO to VPN (the waker).
    const VPN: Token = Token(0);
    const STDIN_WAKER: Token = Token(1);

    // We must use a waker to receive messages from the STDIN-thread while the main-thread is busy
    // reading/connecting to the VPN client.
    let waker = Arc::new(Waker::new(poll.registry(), STDIN_WAKER).unwrap());
    let waker_cloned = waker.clone();
    thread::spawn(move || {
        let mut r = io::Reader::new();
        loop {
            match r.read_input(std::io::stdin()) {
                Some(value) => {
                    match commands::handle(&value){
                        Ok(true) =>{
                            // Command was handled successfully.
                        }
                        _ =>{
                            // For a real message, we wake up the main thread.
                            sender
                                .send(value)
                                .expect("Unable to send data to the main thread");
                            waker_cloned.wake().expect("Unable to wake the main thread");
                        }
                    }
                }
                None => {
                    thread::sleep(time::Duration::from_millis(500));
                }
            }
        }
    });

    // VPN connection status
    let mut vpn_connected = false;

    // This first loop tries to connect to the VPN over and over again.
    loop {
        match TcpStream::connect(SERVER_AND_PORT.parse().unwrap()) {
            Ok(mut stream) => {
                poll.registry()
                    .register(&mut stream, VPN, Interest::READABLE | Interest::WRITABLE)
                    .unwrap();

                let mut r = io::Reader::new();

                // This second loop processes messages coming from the tcp stream and from the
                // STDIN thread via the waker/sender.
                loop {
                    poll.poll(&mut events, None).unwrap();

                    let mut new_connection_needed = false;
                    for event in &events {
                        match event.token() {
                            VPN => {
                                if event.is_error()
                                    || event.is_read_closed()
                                    || event.is_write_closed()
                                {
                                    poll.registry().deregister(&mut stream).unwrap();
                                    vpn_connected = false;
                                    io::write_vpn_down(false);
                                    thread::sleep(time::Duration::from_millis(500));
                                    new_connection_needed = true;
                                }

                                if new_connection_needed {
                                    // We can ignore all the following events
                                    continue;
                                }

                                if !vpn_connected && (event.is_readable() || event.is_writable()) {
                                    vpn_connected = true;
                                    io::write_vpn_up();
                                }

                                if event.is_readable() {
                                    // Until we are able to read things from the socket...
                                    loop {
                                        match r.read_input(&mut stream) {
                                            Some(value) => {
                                                io::write_output(std::io::stdout(), &value)
                                                    .expect("Unable to write to STDOUT");
                                            }
                                            _ => {
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            STDIN_WAKER => {
                                let value = receiver.recv().unwrap();
                                if let Err(_) = io::write_output(&mut stream, &value) {
                                    io::write_vpn_down(true);
                                }
                            }
                            _ => unreachable!(),
                        }
                    }

                    if new_connection_needed {
                        break;
                    }
                }
            }
            _ => {
                thread::sleep(time::Duration::from_millis(500));
            }
        }
    }
}
