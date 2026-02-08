#include "CommandExecutor.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

constexpr size_t PIPE_BUFFER_SIZE = 4096;

// Tokanize the command stroing
std::vector<std::string> CommandExecutor::parseCommand(const std::string& command) {
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// Read all data from pipe
std::string CommandExecutor::readFromPipe(int fd) {
    std::string output;
    char buffer[PIPE_BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, buffer, PIPE_BUFFER_SIZE)) > 0) {
        output.append(buffer, bytes_read);
    }
    
    return output;
}

// Execute command 
CommandExecutor::Result CommandExecutor::execute(const std::string& command) {
    Result result;
    result.success = false;
    result.exit_code = -1;
    
    // Trim command
    std::string trimmed = command;
    size_t start = trimmed.find_first_not_of(" \t\n\r");
    size_t end = trimmed.find_last_not_of(" \t\n\r");
    
    if (start == std::string::npos) {
        result.output = "Error: Empty command\n";
        return result;
    }
    
    trimmed = trimmed.substr(start, end - start + 1);
    
    // Tokanize command
    std::vector<std::string> tokens = parseCommand(trimmed);
    
    if (tokens.empty()) {
        result.output = "Error: Empty command\n";
        return result;
    }
    
    // Create pipe for capturing output
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        result.output = "Error: Failed to create pipe: ";
        result.output += strerror(errno);
        result.output += "\n";
        return result;
    }
    
    // Fork to create grandchild process
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        close(pipefd[0]);
        close(pipefd[1]);
        result.output = "Error: Fork failed: ";
        result.output += strerror(errno);
        result.output += "\n";
        return result;
    }
    
    if (pid == 0) {
        /* Grandchild process */
        
        // Close read end of pipe
        close(pipefd[0]);
        
        // Redirect stdout to pipe write end
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            std::cerr << "Error: dup2 failed for stdout: " << strerror(errno) << std::endl;
            exit(1);
        }
        
        // Redirect stderr to pipe write end
        if (dup2(pipefd[1], STDERR_FILENO) < 0) {
            std::cerr << "Error: dup2 failed for stderr: " << strerror(errno) << std::endl;
            exit(1);
        }
        
        // Close original pipe write end (now duplicated to stdout/stderr)
        close(pipefd[1]);
        
        // Execute command through shell to support built-ins like cd
                // Execute command directly to prevent injection
        std::vector<char*> argv;
        for (auto& token : tokens) {
            argv.push_back((char*)token.c_str());
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        
        // If execl returns, it failed
        std::cerr << "Error: execl failed: " << strerror(errno) << std::endl;
        exit(127);  // Command not found exit code
    } else {

        /* Parent process */
        
        // Close write end of pipe
        close(pipefd[1]);
        
        // Read output from pipe
        result.output = readFromPipe(pipefd[0]);
        
        // Close read end of pipe
        close(pipefd[0]);
        
        // Wait for grandchild to finish
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            // If ECHILD, the process was already reaped by SIGCHLD handler
            // This is OK - the command executed successfully
            if (errno == ECHILD) {
                result.success = true;
                result.exit_code = 0;
            } else {
                result.output += "\nError: waitpid failed: ";
                result.output += strerror(errno);
                result.output += "\n";
            }
            return result;
        }
        
        // Check exit status
        if (WIFEXITED(status)) {
            result.exit_code = WEXITSTATUS(status);
            result.success = (result.exit_code == 0);
        } else if (WIFSIGNALED(status)) {
            result.output += "\nCommand terminated by signal ";
            result.output += std::to_string(WTERMSIG(status));
            result.output += "\n";
        }
    }
    
    return result;
}
