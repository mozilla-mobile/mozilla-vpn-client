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

pub fn write_output<W: Write>(mut output: W, value: &Value) -> Result<(), std::io::Error> {
    let msg = serde_json::to_string(value)?;
    let len = msg.len();
    output.write_u32::<NativeEndian>(len as u32)?;
    output.write_all(msg.as_bytes())?;
    output.flush()?;
    Ok(())
}

pub fn write_vpn_down(error: bool) {
    let field = if error { "error" } else { "status" };
    let value = json!({field: "vpn-client-down"});
    write_output(std::io::stdout(), &value).expect("Unable to write to STDOUT");
}

pub fn write_vpn_up() {
    let value = json!({"status": "vpn-client-up"});
    write_output(std::io::stdout(), &value).expect("Unable to write to STDOUT");
}
