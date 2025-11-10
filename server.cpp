#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>

// Global server socket for graceful shutdown
int server_fd = -1;

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    std::cout << "\nShutting down server..." << std::endl;
    if (server_fd != -1) {
        close(server_fd);
    }
    exit(0);
}

class SimpleWebServer {
private:
    int port;
    struct sockaddr_in address;
    
    std::string createHttpResponse(const std::string& body) {
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html; charset=UTF-8\r\n";
        response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += body;
        return response;
    }

public:
    SimpleWebServer(int p) : port(p) {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
    }

    void start() {
        // Create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Set socket options to reuse address
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Bind socket to port
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Failed to bind to port " << port << std::endl;
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Listen for connections
        if (listen(server_fd, 10) < 0) {
            std::cerr << "Failed to listen on socket" << std::endl;
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        std::cout << "Server is listening on port " << port << "..." << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;

        // Main server loop
        while (true) {
            int client_socket;
            struct sockaddr_in client_address;
            socklen_t client_len = sizeof(client_address);

            // Accept incoming connection
            client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
            if (client_socket < 0) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            // Read request (we don't process it in this simple server)
            char buffer[4096] = {0};
            read(client_socket, buffer, sizeof(buffer) - 1);

            // Log the request (first line only)
            std::string request(buffer);
            size_t first_line_end = request.find("\r\n");
            if (first_line_end != std::string::npos) {
                std::cout << "Request: " << request.substr(0, first_line_end) << std::endl;
            }

            // Create and send response
            std::string html_body = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Simple C++ Web Server</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .container {
            text-align: center;
            background: white;
            padding: 50px;
            border-radius: 10px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.2);
        }
        h1 {
            color: #333;
            margin: 0 0 20px 0;
        }
        p {
            color: #666;
            font-size: 18px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Hello, World!</h1>
        <p>Welcome to the Simple C++ Web Server</p>
        <p>This server is running and responding to your requests!</p>
    </div>
</body>
</html>
)";

            std::string response = createHttpResponse(html_body);
            send(client_socket, response.c_str(), response.length(), 0);

            // Close client connection
            close(client_socket);
        }
    }

    ~SimpleWebServer() {
        if (server_fd != -1) {
            close(server_fd);
        }
    }
};

int main(int argc, char* argv[]) {
    // Register signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Default port
    int port = 8080;

    // Parse command line arguments
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
            if (port < 1 || port > 65535) {
                std::cerr << "Port must be between 1 and 65535" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number. Using default port 8080" << std::endl;
            port = 8080;
        }
    }

    std::cout << "Starting Simple C++ Web Server..." << std::endl;
    
    SimpleWebServer server(port);
    server.start();

    return 0;
}
