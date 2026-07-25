/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use clap::Parser;
use ureq;
use ring::digest;
use std::{io, process};
use std::io::Write;
use x509_parser::time::ASN1Time;

use signature::{Balrog, BalrogError, parse_pem_chain};

/// Program arguments
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Balrog update URL
    url: String,
}

struct BalrogData {
    x5u: String,
    signature: String,
    payload: Vec<u8>,
    chain: Vec<u8>,
}

fn fetch(args: &Args) -> Result<BalrogData, ureq::Error> {
    // Step 1: Fetch the Balrog update details
    let mut response = ureq::get(args.url.clone()).call()?;
    let body = response.body_mut().read_to_vec()?;
    let signature = match response.headers().get("Content-Signature") {
        Some(x) => x.to_str().unwrap(),
        None => return Err(ureq::Error::StatusCode(404)),
    };

    // We should be able to split the content signature on semicolons to get name=value pairs.
    let mut x5u = String::new();
    let mut sigdata = String::new();
    for entry in signature.split(";") {
        let Some((name, value)) = entry.split_once("=") else {
            continue;
        };
        let name = name.trim();
        let value = value.trim();
        if name == "x5u" {
            x5u = value.to_string();
        }
        if name == "p384ecdsa" {
            sigdata = value.to_string();
        }
    }

    // Step 2: Fetch the certificate chain.
    let chain = ureq::get(x5u.clone())
        .call()?
        .body_mut()
        .read_to_vec()?;

    Ok(BalrogData{
        x5u: String::from(x5u),
        payload: body.clone(),
        chain: chain.clone(),
        signature: String::from(sigdata),
    })
}

fn run(data: &BalrogData) -> Result<(), BalrogError> {
    let chain = parse_pem_chain(data.chain.as_slice())?;
    let balrog = Balrog::new(&chain)?;

    // Print some details about the certificate chain.
    if let Some(leaf) = balrog.chain.first() {
        eprintln!("Leaf subject: {}", leaf.subject());
        eprintln!("Leaf expiration: {}", leaf.validity().not_after);
        if let Some(ext) = leaf.subject_alternative_name()? {
            for san in ext.value.general_names.iter() {
                eprintln!("Leaf alternative name: {}", san);
            }
        }
    }
    if balrog.chain.len() >= 2 {
        let root = balrog.chain.last().unwrap();
        eprintln!("Root subject: {}", root.subject());
        eprintln!("Root expiration: {}", root.validity().not_after);
        if let Some(ext) = root.subject_alternative_name()? {
            for san in ext.value.general_names.iter() {
                eprintln!("Root alternative name: {}", san);
            }
        }

        let digest = digest::digest(&digest::SHA256, root.as_raw());
        eprintln!("Root digest: {}", hex::encode(digest));
    } 

    balrog.verify(
        data.payload.as_slice(),
        data.signature.as_str(),
        ASN1Time::now().timestamp(),
        "aus.content-signature.mozilla.org",
    )
}

fn main() {
    let args = Args::parse();

    // Fetch the Balrog update details
    let data = match fetch(&args) {
        Ok(x) => {
            eprintln!("Balrog fetch successful");
            eprintln!("Chain: {}", x.x5u);
            eprintln!("Signature: {}", x.signature);
            eprintln!("");
            x
        },
        Err(e) => {
            eprintln!("Balrog fetch failed: {}", e);
            process::exit(1);
        }
    };

    // Run the cryptographic validation
    let result = run(&data);

    // Write the payload data to stdout.
    let _ = io::stdout().write_all(data.payload.as_slice());

    if result.is_ok() {
        eprintln!("Verification successful");
    } else {
        eprintln!("Verification failed: {}", result.unwrap_err());
        process::exit(1);
    }
}
