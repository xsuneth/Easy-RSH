#include "Socket.h"
#include <stdexcept>
#include <cstring>
#include <fcntl.h>

// Default constructor
Socket::Socket() : fd_(-1), ctx_(nullptr), ssl_(nullptr), use_tls_(false) {}

// Constructor with existing file descriptor
Socket::Socket(int fd) : fd_(fd), ctx_(nullptr), ssl_(nullptr), use_tls_(false) {}

// Destructor
Socket::~Socket() {
    if (ssl_) SSL_free(ssl_);
    if (ctx_) SSL_CTX_free(ctx_);
    close();
}

// Move constructor
Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

// Move assignment
Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

// Get file descriptor
int Socket::get() const {
    return fd_;
}

// Check validity
bool Socket::isValid() const {
    return fd_ >= 0;
}

// Close socket
void Socket::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

// Reset with new file descriptor
void Socket::reset(int new_fd) {
    close();
    fd_ = new_fd;
}

// Create new TCP socket
void Socket::create() {
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
        throw std::runtime_error(std::string("Failed to create socket: ") + strerror(errno));
    }
}

// Bind to port
void Socket::bind(int port) {
    if (!isValid()) {
        throw std::runtime_error("Cannot bind invalid socket");
    }
    
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error(std::string("Failed to bind socket: ") + strerror(errno));
    }
}

// Listen for connections
void Socket::listen(int backlog) {
    if (!isValid()) {
        throw std::runtime_error("Cannot listen on invalid socket");
    }
    
    if (::listen(fd_, backlog) < 0) {
        throw std::runtime_error(std::string("Failed to listen: ") + strerror(errno));
    }
}

// Accept connection
Socket Socket::accept(sockaddr_in& client_addr) {
    if (!isValid()) {
        throw std::runtime_error("Cannot accept on invalid socket");
    }
    
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(fd_, (struct sockaddr*)&client_addr, &addr_len);
    
    if (client_fd < 0) {
        throw std::runtime_error(std::string("Failed to accept connection: ") + strerror(errno));
    }
    
    return Socket(client_fd);
}

// Connect to server
void Socket::connect(const std::string& host, int port) {
    if (!isValid()) {
        throw std::runtime_error("Cannot connect with invalid socket");
    }
    
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert host string to network address
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address: " + host);
    }
    
    if (::connect(fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error(std::string("Failed to connect: ") + strerror(errno));
    }
}

// Send data
ssize_t Socket::send(const void* buffer, size_t length, int flags) {
    if (!isValid()) {
        throw std::runtime_error("Cannot send on invalid socket");
    }
    
    ssize_t sent;
    if (use_tls_ && ssl_) {
        sent = SSL_write(ssl_, (void*)buffer, length);
    } else {
        sent = ::send(fd_, buffer, length, flags);
    }
    if (sent < 0) {
        throw std::runtime_error(std::string("Failed to send: ") + strerror(errno));
    }
    
    return sent;
}

// Receive data
ssize_t Socket::recv(void* buffer, size_t length, int flags) {
    if (!isValid()) {
        throw std::runtime_error("Cannot receive on invalid socket");
    }
    
    ssize_t received;
    if (use_tls_ && ssl_) {
        received = SSL_read(ssl_, buffer, length);
    } else {
        received = ::recv(fd_, buffer, length, flags);
    }
    if (received < 0) {
        throw std::runtime_error(std::string("Failed to receive: ") + strerror(errno));
    }
    
    return received;
}

// Set SO_REUSEADDR option
void Socket::setReuseAddr(bool reuse) {
    if (!isValid()) {
        throw std::runtime_error("Cannot set option on invalid socket");
    }
    
    int opt = reuse ? 1 : 0;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error(std::string("Failed to set SO_REUSEADDR: ") + strerror(errno));
    }
}

// Set non-blocking mode
void Socket::setNonBlocking(bool nonblocking) {
    if (!isValid()) {
        throw std::runtime_error("Cannot set option on invalid socket");
    }
    
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error(std::string("Failed to get socket flags: ") + strerror(errno));
    }
    
    if (nonblocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    if (fcntl(fd_, F_SETFL, flags) < 0) {
        throw std::runtime_error(std::string("Failed to set socket flags: ") + strerror(errno));
    }
}

// TLS/SSL implementation
void Socket::enableTLS(bool is_server) {
    use_tls_ = true;
    
    // Initialize OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    
    // Create SSL context
    const SSL_METHOD* method = is_server ? TLS_server_method() : TLS_client_method();
    ctx_ = SSL_CTX_new(method);
    if (!ctx_) {
        throw std::runtime_error("Failed to create SSL context");
    }
    
    // Set minimum TLS version
    SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);
}

void Socket::loadCertificates(const std::string& cert_file, const std::string& key_file) {
    if (!ctx_) {
        throw std::runtime_error("TLS not enabled, call enableTLS first");
    }
    
    // Load certificate
    if (SSL_CTX_use_certificate_file(ctx_, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error("Failed to load certificate file: " + cert_file);
    }
    
    // Load private key
    if (SSL_CTX_use_PrivateKey_file(ctx_, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error("Failed to load private key file: " + key_file);
    }
    
    // Verify private key
    if (!SSL_CTX_check_private_key(ctx_)) {
        throw std::runtime_error("Private key does not match certificate");
    }
}

void Socket::handshake() {
    if (!ctx_) {
        throw std::runtime_error("TLS not enabled, call enableTLS first");
    }
    
    // Create SSL object
    ssl_ = SSL_new(ctx_);
    if (!ssl_) {
        throw std::runtime_error("Failed to create SSL object");
    }
    
    // Attach SSL to socket
    SSL_set_fd(ssl_, fd_);
    
    // Perform handshake
    int result;
    if (ctx_ && SSL_CTX_get_verify_mode(ctx_) == SSL_VERIFY_PEER) {
        // Client mode
        result = SSL_connect(ssl_);
    } else {
        // Server mode
        result = SSL_accept(ssl_);
    }
    
    if (result <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("TLS handshake failed");
    }
}
