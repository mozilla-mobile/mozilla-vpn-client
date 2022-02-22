/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use chrome_native_messaging::{read_input, write_output};
use mio::net::TcpStream;
use mio::{Events, Interest, Poll, Token, Waker};
use serde_json::json;
use std::sync::mpsc::channel;
use std::sync::Arc;
use std::{thread, time};

const SERVER_AND_PORT: &str = "127.0.0.1:8754";

fn write_vpn_down(error: bool) {
    let field = if error  { "error" }else {"status" };
    let value = json!({field: "vpn-client-down"});
    write_output(std::io::stdout(), &value).expect("Unable to write to STDOUT");
}

fn write_vpn_up() {
    let value = json!({"status": "vpn-client-up"});
    write_output(std::io::stdout(), &value).expect("Unable to write to STDOUT");
}

fn main() {
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
    thread::spawn(move || loop {
        let value = read_input(std::io::stdin()).expect("Failed to read data from STDIN");
        if value == "bridge_ping" {
            // A simple ping/pong message.
            let pong_value = json!("bridge_pong");
            write_output(std::io::stdout(), &pong_value).expect("Unable to write to STDOUT");
        } else {
            // For a real message, we wake up the main thread.
            sender
                .send(value)
                .expect("Unable to send data to the main thread");
            waker_cloned.wake().expect("Unable to wake the main thread");
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
                // This second loop processes messages coming from the tcp stream and from the
                // STDIN thread via the waker/sender.
                'connected: loop {
                    poll.poll(&mut events, None).unwrap();

                    for event in events.iter() {
                        match event.token() {
                            VPN => {
                                if event.is_error()
                                    || event.is_read_closed()
                                    || event.is_write_closed()
                                {
                                    eprintln!("VPN connection dropped");
                                    poll.registry().deregister(&mut stream).unwrap();
                                    vpn_connected = false;
                                    write_vpn_down(false);
                                    thread::sleep(time::Duration::from_millis(500));
                                    break 'connected;
                                }

                                if !vpn_connected && (event.is_readable() || event.is_writable()) {
                                    vpn_connected = true;
                                    write_vpn_up();
                                }

                                if event.is_readable() {
                                    loop {
                                        match read_input(&mut stream) {
                                            Ok(value) => {
                                                write_output(std::io::stdout(), &value)
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
                                match write_output(&mut stream, &value) {
                                    Ok(_) => {}
                                    _ => {
                                        write_vpn_down(true);
                                    }
                                }
                            }
                            _ => unreachable!(),
                        }
                    }
                }
            }
            _ => {
                thread::sleep(time::Duration::from_millis(500));
                continue;
            }
        }
    }
}
