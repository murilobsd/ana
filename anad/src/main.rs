use daemonize::Daemonize;
use std::fs::{self, File};
use std::io::{Read, Write};
use std::os::unix::net::{UnixListener, UnixStream};

fn handle_client(mut stream: UnixStream) {
    let mut buffer = [0; 1024];

    loop {
        match stream.read(&mut buffer) {
            Ok(n) if n > 0 => {
                let received = String::from_utf8_lossy(&buffer[..n]);
                println!("Received command: {}", received);

                // Responder ao comando
                let response =
                    format!("Command '{}' received!", received.trim());
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

    if fs::metadata(socket_path).is_ok() {
        fs::remove_file(socket_path)?;
        println!("Existing socket file removed: {}", socket_path);
    }

    let stdout = File::create("/tmp/anad.out").unwrap();
    let stderr = File::create("/tmp/anad.err").unwrap();

    let daemonize = Daemonize::new()
        .pid_file("/tmp/anad.pid")
        .chown_pid_file(true)
        .working_directory("/tmp")
        .user("nobody")
        .group("nobody")
        .stdout(stdout)
        .stderr(stderr);

    match daemonize.start() {
        Ok(_) => println!("Daemon started successfully."),
        Err(e) => eprintln!("Error starting daemon: {}", e),
    }

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
