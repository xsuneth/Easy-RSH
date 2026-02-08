#include "Auth.h"
#include <crypt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sys/stat.h>

// Constructor
Auth::Auth(const std::string& users_file, int timeout_minutes)
    : users_file_(users_file), session_timeout_minutes_(timeout_minutes) {
    loadUsers();
}

// Load users from file
void Auth::loadUsers() {
    std::ifstream file(users_file_);
    
    if (!file.is_open()) {
        std::cerr << "Warning: Users file not found: " << users_file_ << std::endl;
        std::cerr << "Will create it when users are added." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse line: username:salt:hash
        std::istringstream iss(line);
        std::string username, salt_and_hash;
        
        if (std::getline(iss, username, ':') && std::getline(iss, salt_and_hash)) {
            users_[username] = salt_and_hash;
        }
    }

    file.close();
    std::cout << "Loaded " << users_.size() << " users from " << users_file_ << std::endl;
}

// Save users to file
void Auth::saveUsers() {
    // Create data directory if it doesn't exist
    mkdir("data", 0755);

    std::ofstream file(users_file_);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open users file for writing: " << users_file_ << std::endl;
        return;
    }

    file << "# User authentication file" << std::endl;
    file << "# Format: username:hash" << std::endl;
    file << "# DO NOT EDIT MANUALLY" << std::endl;
    file << std::endl;

    for (const auto& [username, salt_and_hash] : users_) {
        file << username << ":" << salt_and_hash << std::endl;
    }

    file.close();
    std::cout << "Saved " << users_.size() << " users to " << users_file_ << std::endl;
}

// Generate random salt (16 bytes)
std::string Auth::generateSalt() {
    unsigned char salt[16];
    
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }

    // Convert to hex string
    std::ostringstream oss;
    for (unsigned char byte : salt) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    return oss.str();
}

// Hash password with salt using SHA-256
std::string Auth::hashPassword(const std::string& password, const std::string& salt) {
    std::string salted = salt + password;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted.c_str()), 
           salted.length(), hash);

    // Convert to hex string
    std::ostringstream oss;
    for (unsigned char byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    return oss.str();
}

// Generate random session token
std::string Auth::generateToken() {
    unsigned char token[32];
    
    if (RAND_bytes(token, sizeof(token)) != 1) {
        throw std::runtime_error("Failed to generate random token");
    }

    // Convert to hex string
    std::ostringstream oss;
    for (unsigned char byte : token) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    return oss.str();
}

// Authenticate user with username and password
std::string Auth::authenticate(const std::string& username, const std::string& password) {
    // Check if user exists
    auto it = users_.find(username);
    if (it == users_.end()) {
        return "";
    }

    // Parse stored salt:hash
    std::string stored = it->second;
    size_t delimiter_pos = stored.find(':');
    
    if (delimiter_pos == std::string::npos) {
        return "";
    }

    std::string salt = stored.substr(0, delimiter_pos);
    std::string stored_hash = stored.substr(delimiter_pos + 1);

    // Hash provided password with stored salt
    std::string computed_hash = hashPassword(password, salt);

    // Compare hashes
    if (computed_hash != stored_hash) {
        return "";
    }

    // Authentication successful - create session
    std::string token = generateToken();
    
    Session session;
    session.username = username;
    session.token = token;
    session.created_at = std::chrono::system_clock::now();
    session.last_activity = session.created_at;
    session.is_valid = true;

    sessions_[token] = session;
    
    return token;
}

// Check if session is expired
bool Auth::isSessionExpired(const Session& session) const {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(
        now - session.last_activity);
    
    return duration.count() >= session_timeout_minutes_;
}

// Validate a session token
bool Auth::validateToken(const std::string& token) {
    auto it = sessions_.find(token);
    
    if (it == sessions_.end()) {
        return false;
    }

    Session& session = it->second;

    if (!session.is_valid) {
        return false;
    }

    if (isSessionExpired(session)) {
        std::cout << "Session expired for user: " << session.username << std::endl;
        session.is_valid = false;
        return false;
    }

    return true;
}

// Get username from session token
std::string Auth::getUsernameFromToken(const std::string& token) {
    auto it = sessions_.find(token);
    
    if (it != sessions_.end()) {
        return it->second.username;
    }
    
    return "";
}

// Revoke/logout a session
void Auth::revokeToken(const std::string& token) {
    auto it = sessions_.find(token);
    
    if (it != sessions_.end()) {
        std::cout << "Session revoked for user: " << it->second.username << std::endl;
        sessions_.erase(it);
    }
}

// Update session activity timestamp
void Auth::updateActivity(const std::string& token) {
    auto it = sessions_.find(token);
    
    if (it != sessions_.end()) {
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

// Clean up expired sessions
void Auth::cleanupExpiredSessions() {
    auto it = sessions_.begin();
    
    while (it != sessions_.end()) {
        if (isSessionExpired(it->second)) {
            std::cout << "Cleaning up expired session for user: " << it->second.username << std::endl;
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

// Add a new user
bool Auth::addUser(const std::string& username, const std::string& password) {
    // Check if user already exists
    if (users_.find(username) != users_.end()) {
        std::cerr << "Error: User '" << username << "' already exists" << std::endl;
        return false;
    }

    // Generate salt and hash password
    std::string salt = generateSalt();
    std::string hash = hashPassword(password, salt);

    // Store as salt:hash
    users_[username] = salt + ":" + hash;

    std::cout << "User '" << username << "' added successfully" << std::endl;
    
    return true;
}

// Get active session count
size_t Auth::getActiveSessionCount() const {
    return sessions_.size();
}
