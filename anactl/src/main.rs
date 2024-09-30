// Copyright (c) 2024 Murilo Ijanc' <mbsd@m0x.ru>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

use std::io::{self, Read, Write};
use std::os::unix::net::UnixStream;
use std::sync::mpsc;
use std::thread;

fn main() {
    let (tx, rx) = mpsc::channel::<String>();

    thread::spawn(move || {
        let socket_path = "/tmp/anad.sock";

        let mut stream = match UnixStream::connect(socket_path) {
            Ok(stream) => stream,
            Err(e) => {
                eprintln!("Failed to connect to UnixStream: {}", e);
                return;
            }
        };

        for command in rx {
            if let Err(e) = stream.write_all(command.as_bytes()) {
                eprintln!("Failed to send to command via UnixStream: {}", e);
            } else {
                let mut buffer = [0; 1024];
                match stream.read(&mut buffer) {
                    Ok(n) if n > 0 => {
                        let response = String::from_utf8_lossy(&buffer[..n]);
                        println!("Response from server: {}", response);
                    }
                    Ok(_) => println!("No response received"),
                    Err(e) => {
                        eprintln!("Failed to read from UnixStream: {}", e)
                    }
                }
            }
        }
    });

    loop {
        print!("> ");
        io::stdout().flush().unwrap();

        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();

        let input = input.trim();
        if input.is_empty() {
            continue;
        }

        if let Err(e) = tx.send(input.to_string()) {
            eprintln!("Failed to send command to thread: {}", e);
            break;
        }

        if input == "/quit" {
            println!("Quiting...");
            break;
        }
    }
}
