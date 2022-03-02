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

const SERVER_AND_PORT: &str = "127.0.0.1:8754";

#[derive(PartialEq)]
enum ReaderState {
    ReadingLength,
    ReadingBuffer,
}

pub struct Reader {
    state: ReaderState,
    buffer: Vec<u8>,
    length: usize,
}

impl Reader {
    pub fn new() -> Reader {
        Reader {
            state: ReaderState::ReadingLength,
            buffer: Vec::new(),
            length: 0,
        }
    }

    pub fn read_input<R: Read>(&mut self, mut input: R) -> Option<Value> {
        // Until we are able to read things from the stream...
        loop {
            if self.state == ReaderState::ReadingLength {
                assert!(self.buffer.len() < size_of::<u32>());

                let mut buffer = vec![0; size_of::<u32>() - self.buffer.len()];
                match input.read(&mut buffer) {
                    Ok(size) => {
                        // Maybe we have read just part of the buffer. Let's append
                        // only what we have been read.
                        buffer.truncate(size);
                        self.buffer.append(&mut buffer);

                        // Not enough data yet.
                        if self.buffer.len() < size_of::<u32>() {
                            continue;
                        }

                        // Let's convert our buffer into a u32.
                        let mut rdr = Cursor::new(&self.buffer);
                        self.length = rdr.read_u32::<NativeEndian>().unwrap() as usize;
                        if self.length == 0 {
                            continue;
                        }

                        self.state = ReaderState::ReadingBuffer;
                        self.buffer = Vec::with_capacity(self.length);
                    }
                    _ => return None,
                }
            }

            if self.state == ReaderState::ReadingBuffer {
                assert!(self.length > 0);
                assert!(self.buffer.len() < self.length);

                let mut buffer = vec![0; self.length - self.buffer.len()];
                match input.read(&mut buffer) {
                    Ok(size) => {
                        // Maybe we have read just part of the buffer. Let's append
                        // only what we have been read.
                        buffer.truncate(size);
                        self.buffer.append(&mut buffer);

                        // Not enough data yet.
                        if self.buffer.len() < self.length {
                            continue;
                        }

                        match serde_json::from_slice(&self.buffer) {
                            Ok(value) => {
                                self.buffer.clear();
                                self.state = ReaderState::ReadingLength;
                                return Some(value);
                            }
                            _ => {
                                self.buffer.clear();
                                self.state = ReaderState::ReadingLength;
                                continue;
                            }
                        }
                    }
                    _ => return None,
                }
            }
        }
    }
}

fn write_output<W: Write>(mut output: W, value: &Value) -> Result<(), std::io::Error> {
    let msg = serde_json::to_string(value)?;
    let len = msg.len();
    output.write_u32::<NativeEndian>(len as u32)?;
    output.write_all(msg.as_bytes())?;
    output.flush()?;
    Ok(())
}

fn write_vpn_down(error: bool) {
    let field = if error { "error" } else { "status" };
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
    thread::spawn(move || {
        let mut r = Reader::new();
        loop {
            match r.read_input(std::io::stdin()) {
                Some(value) => {
                    if value == "bridge_ping" {
                        // A simple ping/pong message.
                        let pong_value = json!("bridge_pong");
                        write_output(std::io::stdout(), &pong_value)
                            .expect("Unable to write to STDOUT");
                    } else {
                        // For a real message, we wake up the main thread.
                        sender
                            .send(value)
                            .expect("Unable to send data to the main thread");
                        waker_cloned.wake().expect("Unable to wake the main thread");
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

                let mut r = Reader::new();

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
                                    write_vpn_down(false);
                                    thread::sleep(time::Duration::from_millis(500));
                                    new_connection_needed = true;
                                }

                                if new_connection_needed {
                                    // We can ignore all the following events
                                    continue;
                                }

                                if !vpn_connected && (event.is_readable() || event.is_writable()) {
                                    vpn_connected = true;
                                    write_vpn_up();
                                }

                                if event.is_readable() {
                                    // Until we are able to read things from the socket...
                                    loop {
                                        match r.read_input(&mut stream) {
                                            Some(value) => {
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
                                if let Err(_) = write_output(&mut stream, &value) {
                                    write_vpn_down(true);
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
