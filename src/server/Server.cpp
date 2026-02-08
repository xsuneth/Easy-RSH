#include "Server.h"
#include "CommandExecutor.h"
#include "Auth.h"
#include "Colors.h"
#include <iostream>
#include <cstring>
#include <sys/wait.h>
#include <signal.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include <chrono>
#include <fcntl.h>

constexpr size_t BUFFER_SIZE = 4096;

// Signal handler for SIGCHLD to reap zombie processes
void sigchldHandler(int sig) {
    (void)sig;  
    
    // Use WNOHANG to make waitpid non-blocking
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
        // Keep reaping until no more dead children
    }
}

Server::Server(int port) 
    : port_(port), running_(false), use_fork_(false), use_tls_(false), 
      auth_(std::make_shared<Auth>()), require_auth_(true), 
      restart_requested_(false), command_mode_(false) {
    // Initialize with current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        current_dir_ = cwd;
    } else {
        current_dir_ = "/";
    }
}

// Start server
void Server::start() {
    listen_socket_.create();
    listen_socket_.setReuseAddr(true);
    listen_socket_.bind(port_);
    listen_socket_.listen(5);
    
    // Enable TLS if configured
    if (use_tls_) {
        listen_socket_.enableTLS(true);  // Server mode
        listen_socket_.loadCertificates(cert_file_, key_file_);
    }
    
    // Get network IP
    struct ifaddrs *ifaddr, *ifa;
    std::vector<std::string> ip_addresses;
    
    if (getifaddrs(&ifaddr) != -1) {
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) continue;
            if (ifa->ifa_addr->sa_family == AF_INET) {
                void* addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, addr, addressBuffer, INET_ADDRSTRLEN);
                std::string ip(addressBuffer);
                if (ip != "127.0.0.1") {
                    ip_addresses.push_back(ip);
                }
            }
        }
        freeifaddrs(ifaddr);
    }
    
    // Simple server info
    std::cout << Color::PURPLE << "Server listening on port " << Color::BG_PURPLE << " " << port_ << " " << Color::RESET << std::endl;
    
    if (!ip_addresses.empty()) {
        std::cout << Color::GRAY << "Network: " << ip_addresses[0] << ":" << port_ << Color::RESET << std::endl;
    }
    std::cout << Color::GRAY << "Local:   127.0.0.1:" << port_ << Color::RESET << std::endl;
    
    if (!ip_addresses.empty()) {
        std::cout << Color::GRAY << "\nConnect: ./client " << ip_addresses[0] << " " << port_ << Color::RESET << std::endl;
    }
    std::cout << std::endl;
}

// Handle single client - echo mode
void Server::handleClientEcho(Socket& client_socket) {
    char buffer[BUFFER_SIZE];
    
    std::cout << Color::DIM << "  Mode: Echo" << Color::RESET << std::endl;
    
    // Perform authentication if required
    std::string auth_token;
    if (require_auth_) {
        std::cout << Color::DIM << "  ⏳ Authenticating..." << Color::RESET << std::flush;
        auth_token = authenticateClient(client_socket);
        std::cout << "\r\033[K";  // Clear line
        
        if (auth_token.empty()) {
            std::cout << Color::ROSE << "  ✖ Authentication failed" << Color::RESET << std::endl;
            return;
        }
        std::cout << Color::GREEN << "  ✔ Authenticated" << Color::RESET << std::endl;
    }
    
    while (true) {
       
        std::memset(buffer, 0, BUFFER_SIZE);  // Clear buffer
        // Receive data from client
        ssize_t bytes_received = client_socket.recv(buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << Color::GRAY << "Client disconnected" << Color::RESET << std::endl;
            } else {
                std::cerr << Color::ROSE << "Error receiving data" << Color::RESET << std::endl;
            }
            break;
        }
        
        
        std::cout << Color::GRAY << "Received: " << Color::RESET << buffer; // Print received 
      
        client_socket.send(buffer, bytes_received, 0);   // Echo back to client
    }
}

// Handle cd command specially
std::string Server::handleCdCommand(const std::string& path) {
    std::string target_path = path;
    
    // Trim whitespace
    size_t start = target_path.find_first_not_of(" \t\n\r");
    size_t end = target_path.find_last_not_of(" \t\n\r");
    if (start != std::string::npos) {
        target_path = target_path.substr(start, end - start + 1);
    }
    
    // Handle empty path (cd with no args goes to home)
    if (target_path.empty()) {
        const char* home = getenv("HOME");
        if (home) {
            target_path = home;
        } else {
            return "cd: HOME not set\n";
        }
    }
    
    // Handle ~ expansion
    if (target_path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            target_path = std::string(home) + target_path.substr(1);
        }
    }
    
    // Handle relative paths
    if (target_path[0] != '/') {
        target_path = current_dir_ + "/" + target_path;
    }
    
    // Try to change directory
    if (chdir(target_path.c_str()) == 0) {
        // Update current directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            current_dir_ = cwd;
            return "";  // Success, no output
        } else {
            return "cd: failed to get current directory\n";
        }
    } else {
        return "cd: " + target_path + ": " + strerror(errno) + "\n";
    }
}

// Handle client - command execution mode 
void Server::handleClientCommand(Socket& client_socket) {
    char buffer[BUFFER_SIZE];
    
    std::cout << Color::DIM << "  Mode: Command Execution" << Color::RESET << std::endl;
    
    // Perform authentication if required
    std::string auth_token;
    if (require_auth_) {
        std::cout << Color::DIM << "  ⏳ Authenticating..." << Color::RESET << std::flush;
        auth_token = authenticateClient(client_socket);
        std::cout << "\r\033[K";  // Clear line
        
        if (auth_token.empty()) {
            std::cout << Color::ROSE << "  ✖ Authentication failed" << Color::RESET << std::endl;
            return;
        }
        std::cout << Color::GREEN << "  ✔ Authenticated" << Color::RESET << std::endl;
    }
    
    while (true) {
        // Clear buffer
        std::memset(buffer, 0, BUFFER_SIZE);
        
        // Receive command from client
        ssize_t bytes_received = client_socket.recv(buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << Color::GRAY << "Client disconnected" << Color::RESET << std::endl;
            } else {
                std::cerr << Color::ROSE << "Error receiving data" << Color::RESET << std::endl;
            }
            break;
        }
        
        std::string command(buffer, bytes_received);
        std::cout << Color::GRAY << "Executing: " << Color::BG_PURPLE << " " << command.substr(0, command.length()-1) << " " << Color::RESET << std::endl;
        
        // Check if it's a cd command
        std::string trimmed = command;
        size_t start = trimmed.find_first_not_of(" \t\n\r");
        if (start != std::string::npos) {
            trimmed = trimmed.substr(start);
        }
        
        std::string response;
        
        if (trimmed.substr(0, 2) == "cd" && (trimmed.length() == 2 || trimmed[2] == ' ' || trimmed[2] == '\t' || trimmed[2] == '\n')) {
            // Handle cd command
            std::string path = trimmed.substr(2);
            std::string cd_result = handleCdCommand(path);
            
            if (cd_result.empty()) {
                // Success - send current directory as confirmation
                response = current_dir_ + "\n";
            } else {
                // Error
                response = cd_result;
            }
        } else if (trimmed == "pwd" || trimmed == "pwd\n") {
            // Handle pwd command
            response = current_dir_ + "\n";
        } else if (trimmed == "remoot" || trimmed == "remoot\n") {
            // Handle server restart command
            response = "Server restart requested. Restarting...\n";
            client_socket.send(response.c_str(), response.length(), 0);
            std::cout << Color::PURPLE << "Restart requested by client. Shutting down for restart..." << Color::RESET << std::endl;
            restart_requested_ = true;
            running_ = false;
            return;
        } else {
            // Make sure we're in the correct directory before executing
            if (chdir(current_dir_.c_str()) != 0) {
                response = "Error: Failed to change to working directory\n";
            } else {
                // Execute command
                CommandExecutor::Result result = CommandExecutor::execute(command);
                
                // Prepare response
                if (!result.output.empty()) {
                    response = result.output;
                } else {
                    response = "(no output)\n";
                }
                
                // Add exit code if command failed
                if (!result.success && result.exit_code >= 0) {
                    response += "[Exit code: " + std::to_string(result.exit_code) + "]\n";
                }
            }
        }
        
        // Send response back to client
        try {
            client_socket.send(response.c_str(), response.length(), 0);
        } catch (const std::exception& e) {
            std::cerr << "Error sending response: " << e.what() << std::endl;
            break;
        }
    }
}

// Main server -  loop
void Server::run() {
    running_ = true;
    
    // Install SIGCHLD handler if using fork
    if (use_fork_) {
        struct sigaction sa;
        sa.sa_handler = sigchldHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;  // Restart interrupted system calls
        
        if (sigaction(SIGCHLD, &sa, nullptr) < 0) {
            std::cerr << "Warning: Failed to install SIGCHLD handler: " 
                      << strerror(errno) << std::endl;
        } else {
            std::cout << "SIGCHLD handler installed for zombie reaping" << std::endl;
        }
    }
    
    while (running_) {
        try {
            sockaddr_in client_addr;
            
            // Animated spinner for waiting
            const char spinner[] = {'/', '-', '\\', '|'};
            int spinner_idx = 0;
            
            // Non-blocking accept with spinner
            std::cout << Color::DIM << "\rWaiting for connection " << spinner[0] << Color::RESET << std::flush;
            
            // Set socket to non-blocking temporarily for spinner
            int flags = fcntl(listen_socket_.get(), F_GETFL, 0);
            fcntl(listen_socket_.get(), F_SETFL, flags | O_NONBLOCK);
            
            Socket client_socket;
            while (running_) {
                try {
                    client_socket = listen_socket_.accept(client_addr);
                    break;  // Connection received!
                } catch (...) {
                    // No connection yet, update spinner
                    spinner_idx = (spinner_idx + 1) % 4;
                    std::cout << "\rWaiting for connection " << Color::DIM << spinner[spinner_idx] << Color::RESET << std::flush;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            
            // Restore blocking mode
            fcntl(listen_socket_.get(), F_SETFL, flags);
            
            if (!running_) break;
            
            // Clear spinner line
            std::cout << "\r\033[K";
            
            // Convert client address to string
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << Color::THEME << "→ " << Color::RESET << "Connection from "
                      << Color::THEME << client_ip << ":" << ntohs(client_addr.sin_port) << Color::RESET << std::endl;
            // Perform TLS handshake if enabled
            // if (use_tls_) {
            //     try {
            //         client_socket.handshake();
            //         std::cout << Color::GREEN << "  🔒 TLS handshake successful" << Color::RESET << std::endl;
            //     } catch (const std::exception& e) {
            //         std::cerr << Color::ROSE << "  ❌ TLS handshake failed: " << e.what() << Color::RESET << std::endl;
            //         continue;
            //     }
            // }
                if (pid < 0) {
                    std::cerr << "Fork failed: " << strerror(errno) << std::endl;
                    // Continue accepting other clients
                    continue;
                }
                
                if (pid == 0) {
                    // Child process
                    listen_socket_.close();  // Child doesn't need listening socket
                    
                    try {
                        if (command_mode_) {
                            handleClientCommand(client_socket);
                        } else {
                            handleClientEcho(client_socket);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error in child process: " << e.what() << std::endl;
                    }
                    exit(0);// Exit child process
                } else {
                    
                    // Parent process
                    client_socket.close();  // Parent doesn't need client socket
                    std::cout << Color::GRAY << "Spawned process (PID: " << pid << ")" << Color::RESET << std::endl;
                }
            } else {
                // Handle client in main process (Phase 1)
                if (command_mode_) {
                    handleClientCommand(client_socket);
                } else {
                    handleClientEcho(client_socket);
                }
            }
                // Handle client in main process (Phase 1)
                if (command_mode_) {
                    handleClientCommand(client_socket);
                } else {
                    handleClientEcho(client_socket);
                }
            }
            
        } catch (const std::exception& e) {
            std::cerr << Color::ROSE << "Error: " << e.what() << Color::RESET << std::endl;
            if (!use_fork_) {
                // continue to accept new connections in single-client mode
                continue;
            }
        }
    }
}

// Stop server
void Server::stop() {
    running_ = false;
    listen_socket_.close();
    std::cout << Color::GRAY << "\nServer stopped." << Color::RESET << std::endl;
}

// Enable or disable fork
void Server::setUseFork(bool use_fork) {
    use_fork_ = use_fork;
    if (use_fork) {
        std::cout << Color::GRAY << "Mode: Multi-client (fork)" << Color::RESET << std::endl;
    } else {
        std::cout << Color::GRAY << "Mode: Single-client" << Color::RESET << std::endl;
    }
}

// Enable or disable command mode
void Server::setCommandMode(bool enable) {
    command_mode_ = enable;
    if (enable) {
        std::cout << Color::GRAY << "Mode: Command execution" << Color::RESET << std::endl;
    } else {
        std::cout << Color::GRAY << "Mode: Echo" << Color::RESET << std::endl;
    }
}

// Enable or disable authentication
void Server::setRequireAuth(bool require) {
    require_auth_ = require;
    if (require) {
        std::cout << "Authentication enabled" << std::endl;
    } else {
        std::cout << "Warning: Authentication disabled - server is insecure!" << std::endl;
    }
}

// Get authentication module
std::shared_ptr<Auth> Server::getAuth() {
    return auth_;
}

// Check if restart was requested
bool Server::isRestartRequested() const {
    return restart_requested_;
}

// Perform authentication handshake with client
std::string Server::authenticateClient(Socket& client_socket) {
    char buffer[BUFFER_SIZE];
    
    // Send authentication request
    const char* auth_prompt = "AUTH_REQUIRED\n";
    client_socket.send(auth_prompt, strlen(auth_prompt), 0);
    
    // Receive credentials (format: "AUTH username:password")
    std::memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_received = client_socket.recv(buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        std::cerr << "Failed to receive authentication credentials" << std::endl;
        return "";
    }
    
    std::string auth_msg(buffer, bytes_received);
    
    // Remove trailing newline
    if (!auth_msg.empty() && auth_msg.back() == '\n') {
        auth_msg.pop_back();
    }
    
    // Parse AUTH command
    if (auth_msg.find("AUTH ") != 0) {
        std::cerr << "Invalid authentication message format" << std::endl;
        const char* error_msg = "AUTH_FAILED Invalid format\n";
        client_socket.send(error_msg, strlen(error_msg), 0);
        return "";
    }
    
    // Extract credentials
    std::string credentials = auth_msg.substr(5); // Skip "AUTH "
    size_t delimiter_pos = credentials.find(':');
    
    if (delimiter_pos == std::string::npos) {
        std::cerr << "Invalid credentials format" << std::endl;
        const char* error_msg = "AUTH_FAILED Invalid credentials format\n";
        client_socket.send(error_msg, strlen(error_msg), 0);
        return "";
    }
    
    std::string username = credentials.substr(0, delimiter_pos);
    std::string password = credentials.substr(delimiter_pos + 1);
    
    // Authenticate
    std::string token = auth_->authenticate(username, password);
    
    if (token.empty()) {
        const char* error_msg = "AUTH_FAILED Invalid username or password\n";
        client_socket.send(error_msg, strlen(error_msg), 0);
        return "";
    }
    
    // Send success with token
    std::string success_msg = "AUTH_SUCCESS " + token + "\n";
    client_socket.send(success_msg.c_str(), success_msg.length(), 0);
    
    return token;
}

// Enable TLS with certificate files
void Server::enableTLS(const std::string& cert_file, const std::string& key_file) {
    use_tls_ = true;
    cert_file_ = cert_file;
    key_file_ = key_file;
    std::cout << Color::GREEN << "TLS enabled - Certificate: " << cert_file 
              << ", Key: " << key_file << Color::RESET << std::endl;
}
