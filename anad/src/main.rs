use std::os::unix::net::{UnixListener, UnixStream};
use std::io::{Read, Write};

fn handle_client(mut stream: UnixStream) {
    let mut buffer = [0; 1024];

    loop {
        match stream.read(&mut buffer) {
            Ok(n) if n > 0 => {
                let received = String::from_utf8_lossy(&buffer[..n]);
                println!("Received command: {}", received);
                
                // Responder ao comando
                let response = format!("Command '{}' received!", received.trim());
                if let Err(e) = stream.write_all(response.as_bytes()) {
                    eprintln!("Failed to send response: {}", e);
                }
            }
            Ok(_) => break,
            Err(e) => {
                eprintln!("Failed to read from stream: {}", e);
                break;
            }
        }
    }
}

fn main() -> std::io::Result<()> {
    let socket_path = "/tmp/anad.sock";
    let listener = UnixListener::bind(socket_path)?;

    println!("Server listening on {}", socket_path);

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                println!("Client connected!");
                handle_client(stream);
            }
            Err(err) => eprintln!("Connection failed: {}", err),
        }
    }

    Ok(())
}

