#include "Client.h"
#include "Colors.h"
#include <iostream>
#include <cstring>
#include <sstream>

constexpr size_t BUFFER_SIZE = 4096;


Client::Client(const std::stringClient::Client(const std::string& host, int port) host, int port) : use_tls_(false)
    : server_host_(host), server_port_(port), connected_(false), use_tls_(false) {
}

// Connect to server
bool Client::connect() {
    try {
        std::cout << Color::GRAY << "Connecting to " << server_host_ << ":" << server_port_ << "..." << Color::RESET << std::endl;
        
        // Create socket
        
        // Enable TLS if configured
        if (use_tls_) {
            socket_.enableTLS(false);  // Client mode
        }
        socket_.create();
        
        // Connect to server
        
        // Perform TLS handshake if enabled
        if (use_tls_) {
            socket_.handshake();
            std::cout << Color::GREEN << "🔒 TLS handshake successful" << Color::RESET << std::endl;
        }
        socket_.connect(server_host_, server_port_);
        
        connected_ = true;

        std::cout << Color::PURPLE << "Connected to " << Color::BG_PURPLE << " " << server_host_ << ":" << server_port_ << " " << Color::RESET << std::endl;
        if (!performAuthentication()) {
            std::cerr << "Authentication failed" << std::endl;
            disconnect();
            return false;
        }
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << Color::ROSE << "Connection failed: " << e.what() << Color::RESET << std::endl;
        connected_ = false;
        return false;
    }
}


void Client::disconnect() {
    if (connected_) {
        socket_.close();
        connected_ = false;
        std::cout << Color::GRAY << "\nDisconnected from server." << Color::RESET << std::endl;
    }
}


std::string Client::sendCommand(const std::string& command) {
    if (!connected_) {
        throw std::runtime_error("Not connected to server");
    }
    
   
    socket_.send(command.c_str(), command.length(), 0);
    
    
    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, BUFFER_SIZE);
    
    ssize_t bytes_received = socket_.recv(buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            std::cout << "Server closed connection." << std::endl;
            connected_ = false;
        }
        return "";
    }
    
    return std::string(buffer, bytes_received);
}


void Client::runInteractiveShell() {
    if (!connected_) {
        std::cerr << Color::ROSE << "Not connected to server. Call connect() first." << Color::RESET << std::endl;
        return;
    }
    
    std::cout << "\n" << Color::GRAY << "Type 'exit' or 'quit' to disconnect." << Color::RESET << "\n" << std::endl;
    
    std::string input;
    
    while (connected_) {
        
        std::cout << Color::PURPLE << Color::BOLD << "remote" << Color::RESET << Color::GRAY << "> " << Color::RESET << std::flush;
        
        
        if (!std::getline(std::cin, input)) {
           
            break;
        }
        
      
        size_t start = input.find_first_not_of(" \t\n\r");
        size_t end = input.find_last_not_of(" \t\n\r");
        
        if (start == std::string::npos) {
            
            continue;
        }
        
        input = input.substr(start, end - start + 1);
        
    
        if (input == "exit" || input == "quit") {
            std::cout << Color::GRAY << "Disconnecting..." << Color::RESET << std::endl;
            break;
        }
     
        if (input.empty()) {
            continue;
        }
        
        try {
            
            std::string command = input + "\n";
            
           
            std::string response = sendCommand(command);
            
            if (!response.empty()) {
                // Add left margin to output for visual separation
                std::string line;
                std::istringstream stream(response);
                while (std::getline(stream, line)) {
                    std::cout << Color::GRAY << "  │ " << Color::RESET << line << std::endl;
                }
            }
            
        } catch (const std::exception& e) {
            std::cerr << Color::ROSE << "Error: " << e.what() << Color::RESET << std::endl;
            break;
        }
    }
    
    disconnect();
}


bool Client::isConnected() const {
    return connected_;
}

// Set authentication credentials
void Client::setCredentials(const std::string& username, const std::string& password) {
    username_ = username;
    password_ = password;
}

// Perform authentication handshake
bool Client::performAuthentication() {
    char buffer[BUFFER_SIZE];
    
    // Wait for AUTH_REQUIRED message
    std::memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_received = socket_.recv(buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        std::cerr << "Failed to receive authentication prompt" << std::endl;
        return false;
    }
    
    std::string prompt(buffer, bytes_received);
    
    // Check if server requires authentication
    if (prompt.find("AUTH_REQUIRED") == std::string::npos) {
        // Server doesn't require auth, we're good
        std::cout << "Server does not require authentication" << std::endl;
        return true;
    }
    
    std::cout << "Server requires authentication" << std::endl;
    
    // Prompt for credentials if not set
    if (username_.empty()) {
        std::cout << "Username: " << std::flush;
        std::getline(std::cin, username_);
    }
    
    if (password_.empty()) {
        std::cout << "Password: " << std::flush;
        // In production, use termios to hide password input
        std::getline(std::cin, password_);
    }
    
    // Send credentials
    std::string auth_msg = "AUTH " + username_ + ":" + password_ + "\n";
    socket_.send(auth_msg.c_str(), auth_msg.length(), 0);
    
    // Receive authentication response
    std::memset(buffer, 0, BUFFER_SIZE);
    bytes_received = socket_.recv(buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        std::cerr << "Failed to receive authentication response" << std::endl;
        return false;
    }
    
    std::string response(buffer, bytes_received);
    
    if (response.find("AUTH_SUCCESS") == 0) {
        // Extract token
        size_t space_pos = response.find(' ');
        if (space_pos != std::string::npos) {
            auth_token_ = response.substr(space_pos + 1);
            // Remove trailing newline
            if (!auth_token_.empty() && auth_token_.back() == '\n') {
                auth_token_.pop_back();
            }
        }
        std::cout << "Authentication successful!" << std::endl;
        return true;
    } else {
        std::cerr << "Authentication failed: " << response;
        return false;
    }
}

// Enable TLS for client connections
void Client::enableTLS() {
    use_tls_ = true;
    std::cout << Color::GREEN << "TLS enabled for client connection" << Color::RESET << std::endl;
}
