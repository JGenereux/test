# Modern C++ Web Server

A simple, modern C++ web server implementation demonstrating best practices and clean code principles.

## Features

- **Modern C++17** implementation
- **RAII-based** resource management
- **Thread-safe** signal handling
- **Graceful shutdown** on SIGINT/SIGTERM
- **Zero warnings** compilation with strict flags
- **Exception-safe** error handling
- **Const correctness** throughout

## Building

### Requirements
- C++17 compliant compiler (GCC 7+, Clang 5+, or equivalent)
- POSIX-compliant system (Linux, macOS, etc.)
- Make

### Build Commands

```bash
# Build release version
make

# Build debug version
make debug

# Clean build artifacts
make clean
```

## Running

```bash
# Run on default port 8080
./server

# Run on custom port
./server 3000

# Or use make
make run                # Port 8080
make run-port          # Interactive port selection
```

Visit `http://localhost:8080` (or your custom port) in a web browser.

## Improvements from Original Code

### 1. **RAII Socket Management**
- Implemented `SocketRAII` class for automatic resource cleanup
- Prevents resource leaks even when exceptions occur
- Move semantics for efficient ownership transfer

### 2. **Eliminated Global Mutable State**
- Replaced global `int server_fd` with atomic boolean flag
- Server socket now managed within class instance
- Thread-safe shutdown signaling using `std::atomic`

### 3. **Modern C++ Features**
- `[[nodiscard]]` attributes for critical return values
- `noexcept` specifications for exception safety guarantees
- `constexpr` for compile-time constants
- Structured bindings and modern initialization

### 4. **Enhanced Error Handling**
- Exception-based error reporting with descriptive messages
- Proper use of `errno` and `std::strerror` for system errors
- Graceful degradation for non-critical failures
- Top-level exception handling in `main()`

### 5. **Const Correctness**
- Member functions marked `const` where appropriate
- Immutable data declared with `const`
- Better compiler optimization opportunities

### 6. **Improved Signal Handling**
- Non-blocking server loop with `select()` and timeout
- Periodic checking of shutdown flag
- Clean shutdown without race conditions
- Handles EINTR interruptions properly

### 7. **Better Code Organization**
- Clear separation of concerns
- Private helper methods
- Static methods for pure functions
- Improved encapsulation

### 8. **Input Validation**
- Port number range checking
- Exception on invalid configuration
- Safe handling of malformed command-line arguments

### 9. **Compiler Warnings**
- Builds with `-Wall -Wextra -Wpedantic` with zero warnings
- Unused parameter annotations
- Proper type conversions

### 10. **Code Style & Readability**
- Consistent naming conventions
- Clear variable names
- Comprehensive comments
- Modern HTML5 with proper semantics

## Architecture

```
┌─────────────────┐
│   main()        │  Entry point, exception handling
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  WebServer      │  Main server class
│  - start()      │  - Manages server lifecycle
│  - handleClient │  - Processes HTTP requests
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  SocketRAII     │  RAII wrapper for socket FDs
│  - close()      │  - Automatic resource cleanup
│  - release()    │  - Move semantics support
└─────────────────┘
```

## Security Considerations

- **No buffer overflows**: Fixed-size buffers with bounds checking
- **Resource limits**: Connection backlog and read buffer size limits
- **Input validation**: Port range and argument validation
- **Clean shutdown**: Proper cleanup prevents resource exhaustion

## Future Enhancements

Potential improvements for production use:
- [ ] Multi-threading with thread pool
- [ ] HTTP request parsing and routing
- [ ] Static file serving
- [ ] Logging framework
- [ ] Configuration file support
- [ ] SSL/TLS support
- [ ] Rate limiting
- [ ] Request/response middleware

## License

This is a demonstration project for educational purposes.
