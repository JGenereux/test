# Simple C++ Web Server

A lightweight HTTP web server written in C++ using POSIX sockets. The server listens on a configurable port and responds with a simple "Hello, World!" HTML page to any HTTP request.

## Features

- **Configurable Port**: Specify the port via command-line argument (default: 8080)
- **Simple HTTP Response**: Serves a beautiful "Hello, World!" HTML page
- **Graceful Shutdown**: Handles Ctrl+C for clean server shutdown
- **Request Logging**: Logs incoming HTTP requests to the console
- **No External Dependencies**: Uses only standard C++ and POSIX libraries

## Build Instructions

### Prerequisites

- C++ compiler (g++ or clang++)
- Linux or Unix-like operating system (macOS, WSL, etc.)

### Compilation

#### Using Make (Recommended)

```bash
make
```

This will compile the server and create an executable named `server`.

#### Manual Compilation

```bash
g++ -std=c++11 -Wall -Wextra -O2 -o server server.cpp
```

### Cleaning Build Artifacts

```bash
make clean
```

## Usage

### Starting the Server

#### Default Port (8080)

```bash
./server
```

#### Custom Port

```bash
./server 3000
```

Replace `3000` with your desired port number (1-65535).

### Accessing the Server

Once the server is running, open your web browser and navigate to:

- `http://localhost:8080` (for default port)
- `http://localhost:3000` (for custom port 3000)

You can also use curl from the command line:

```bash
curl http://localhost:8080
```

### Stopping the Server

Press `Ctrl+C` in the terminal to gracefully shut down the server.

## Example Output

```
Starting Simple C++ Web Server...
Server is listening on port 8080...
Press Ctrl+C to stop the server
Request: GET / HTTP/1.1
Request: GET /favicon.ico HTTP/1.1
^C
Shutting down server...
```

## Technical Details

- **Protocol**: HTTP/1.1
- **Socket Type**: TCP (SOCK_STREAM)
- **Address Family**: IPv4 (AF_INET)
- **Binding**: INADDR_ANY (0.0.0.0) - accepts connections on all network interfaces
- **Backlog**: 10 pending connections

## Troubleshooting

### Port Already in Use

If you see an error like "Failed to bind to port", the port is already in use. Try:
- Using a different port number
- Stopping any other process using that port
- Waiting a few seconds if you just stopped the server

### Permission Denied

Ports below 1024 require root privileges. Either:
- Use a port number ≥ 1024 (e.g., 8080)
- Run with sudo: `sudo ./server 80` (not recommended for development)

## License

This is free and unencumbered software released into the public domain.
