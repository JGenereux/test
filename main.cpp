#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <csignal>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// RAII wrapper for socket file descriptors
class SocketRAII {
private:
    int fd_;

public:
    explicit SocketRAII(int fd = -1) : fd_(fd) {}
    
    ~SocketRAII() {
        close();
    }
    
    // Disable copying
    SocketRAII(const SocketRAII&) = delete;
    SocketRAII& operator=(const SocketRAII&) = delete;
    
    // Enable moving
    SocketRAII(SocketRAII&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    
    SocketRAII& operator=(SocketRAII&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    
    [[nodiscard]] int get() const noexcept { return fd_; }
    
    [[nodiscard]] bool isValid() const noexcept { return fd_ >= 0; }
    
    void close() noexcept {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }
    
    // Transfer ownership
    [[nodiscard]] int release() noexcept {
        int tmp = fd_;
        fd_ = -1;
        return tmp;
    }
};

// Global atomic flag for graceful shutdown
std::atomic<bool> shutdown_requested{false};

// Signal handler for graceful shutdown
void signalHandler(int /* signum */) {
    shutdown_requested.store(true, std::memory_order_release);
}

class WebServer {
private:
    int port_;
    SocketRAII server_socket_;
    
    [[nodiscard]] static std::string createHttpResponse(const std::string& body) {
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html; charset=UTF-8\r\n";
        response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += body;
        return response;
    }
    
    [[nodiscard]] static std::string getHtmlContent() {
        return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Simple C++ Web Server</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .container {
            text-align: center;
            background: white;
            padding: 3rem;
            border-radius: 1rem;
            box-shadow: 0 1rem 2rem rgba(0, 0, 0, 0.2);
            max-width: 600px;
        }
        h1 {
            color: #333;
            margin: 0 0 1.5rem 0;
            font-size: 2.5rem;
        }
        p {
            color: #666;
            font-size: 1.125rem;
            line-height: 1.6;
            margin: 0.5rem 0;
        }
        .badge {
            display: inline-block;
            background: #667eea;
            color: white;
            padding: 0.5rem 1rem;
            border-radius: 0.5rem;
            margin-top: 1rem;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 Hello, World!</h1>
        <p>Welcome to the Modern C++ Web Server</p>
        <p>This server is running with modern C++17 features!</p>
        <div class="badge">Status: Active ✓</div>
    </div>
</body>
</html>)";
    }
    
    void handleClient(SocketRAII client_socket) {
        if (!client_socket.isValid()) {
            std::cerr << "Invalid client socket" << std::endl;
            return;
        }
        
        // Read request
        constexpr size_t BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE] = {0};
        const ssize_t bytes_read = read(client_socket.get(), buffer, BUFFER_SIZE - 1);
        
        if (bytes_read < 0) {
            std::cerr << "Failed to read from client socket" << std::endl;
            return;
        }
        
        // Log the request (first line only)
        const std::string request(buffer, static_cast<size_t>(bytes_read));
        const size_t first_line_end = request.find("\r\n");
        if (first_line_end != std::string::npos) {
            std::cout << "Request: " << request.substr(0, first_line_end) << std::endl;
        }
        
        // Create and send response
        const std::string html_body = getHtmlContent();
        const std::string response = createHttpResponse(html_body);
        
        const ssize_t bytes_sent = send(client_socket.get(), response.c_str(), 
                                       response.length(), 0);
        if (bytes_sent < 0) {
            std::cerr << "Failed to send response to client" << std::endl;
        }
    }

public:
    explicit WebServer(int port) : port_(port) {
        if (port < 1 || port > 65535) {
            throw std::invalid_argument("Port must be between 1 and 65535");
        }
    }
    
    void start() {
        // Create socket
        const int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            throw std::runtime_error("Failed to create socket: " + 
                                    std::string(std::strerror(errno)));
        }
        server_socket_ = SocketRAII(fd);
        
        // Set socket options to reuse address
        const int opt = 1;
        if (setsockopt(server_socket_.get(), SOL_SOCKET, SO_REUSEADDR, 
                      &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("Failed to set socket options: " + 
                                    std::string(std::strerror(errno)));
        }
        
        // Bind socket to port
        struct sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(static_cast<uint16_t>(port_));
        
        if (bind(server_socket_.get(), reinterpret_cast<struct sockaddr*>(&address), 
                sizeof(address)) < 0) {
            throw std::runtime_error("Failed to bind to port " + std::to_string(port_) + 
                                    ": " + std::string(std::strerror(errno)));
        }
        
        // Listen for connections
        constexpr int BACKLOG = 10;
        if (listen(server_socket_.get(), BACKLOG) < 0) {
            throw std::runtime_error("Failed to listen on socket: " + 
                                    std::string(std::strerror(errno)));
        }
        
        std::cout << "Server listening on http://localhost:" << port_ << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // Main server loop
        while (!shutdown_requested.load(std::memory_order_acquire)) {
            struct sockaddr_in client_address{};
            socklen_t client_len = sizeof(client_address);
            
            // Accept incoming connection (with timeout for checking shutdown flag)
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(server_socket_.get(), &read_fds);
            
            struct timeval timeout{1, 0};  // 1 second timeout
            const int activity = select(server_socket_.get() + 1, &read_fds, 
                                       nullptr, nullptr, &timeout);
            
            if (activity < 0) {
                if (errno == EINTR) {
                    continue;  // Interrupted by signal, check shutdown flag
                }
                std::cerr << "Select error: " << std::strerror(errno) << std::endl;
                continue;
            }
            
            if (activity == 0) {
                continue;  // Timeout, check shutdown flag
            }
            
            const int client_fd = accept(server_socket_.get(), 
                                        reinterpret_cast<struct sockaddr*>(&client_address), 
                                        &client_len);
            if (client_fd < 0) {
                if (errno == EINTR) {
                    continue;  // Interrupted by signal
                }
                std::cerr << "Failed to accept connection: " << std::strerror(errno) << std::endl;
                continue;
            }
            
            handleClient(SocketRAII(client_fd));
        }
        
        std::cout << "\nShutting down server gracefully..." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    try {
        // Register signal handlers for graceful shutdown
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // Default port
        int port = 8080;
        
        // Parse command line arguments
        if (argc > 1) {
            try {
                port = std::stoi(argv[1]);
            } catch (const std::exception& e) {
                std::cerr << "Invalid port number: " << argv[1] 
                         << ". Using default port 8080" << std::endl;
                port = 8080;
            }
        }
        
        std::cout << "Starting Modern C++ Web Server..." << std::endl;
        
        WebServer server(port);
        server.start();
        
        return EXIT_SUCCESS;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return EXIT_FAILURE;
    }
}
