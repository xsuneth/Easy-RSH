#include "Server.h"
#include "Config.h"
#include "SetupWizard.h"
#include "CLIUtils.h"
#include "Colors.h"
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <unistd.h>

// ASCII Banner (minimal)
void printBanner() {
    std::cout << "\n";
    std::cout << Color::THEME << Color::BOLD;
    std::cout << "  ███████╗ █████╗ ███████╗██╗   ██╗    ██████╗ ███████╗██╗  ██╗\n";
    std::cout << "  ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝    ██╔══██╗██╔════╝██║  ██║\n";
    std::cout << "  █████╗  ███████║███████╗ ╚████╔╝     ██████╔╝███████╗███████║\n";
    std::cout << "  ██╔══╝  ██╔══██║╚════██║  ╚██╔╝      ██╔══██╗╚════██║██╔══██║\n";
    std::cout << "  ███████╗██║  ██║███████║   ██║       ██║  ██║███████║██║  ██║\n";
    std::cout << "  ╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝       ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n";
    std::cout << Color::RESET;
    std::cout << Color::DIM << "                    Easy Remote Shell Server v1.1.0" << Color::RESET << "\n\n";
}

// Global server 
Server* g_server = nullptr;

// Signal handler 
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(signum);
}

int main(int argc, char* argv[]) {
    try {
        // Clear terminal at start
        CLI::clear();
        
        // Check for command-line flags
        bool reconfigure = false;
        int port_override = -1;
        bool fork_override = false;
        bool command_override = false;
        bool tls_override = false;
        std::string cert_file, key_file;
        bool has_overrides = false;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "--reconfigure") {
                reconfigure = true;
            } else if (arg == "-p" || arg == "--port") {
                if (i + 1 < argc) {
                    port_override = std::atoi(argv[++i]);
                    has_overrides = true;
                }
            } else if (arg == "-f" || arg == "--fork") {
                fork_override = true;
                has_overrides = true;
            } else if (arg == "-c" || arg == "--command") {
            } else if (arg == "--tls") {
                tls_override = true;
                has_overrides = true;
            } else if (arg == "--cert") {
                if (i + 1 < argc) {
                    cert_file = argv[++i];
                    has_overrides = true;
                }
            } else if (arg == "--key") {
                if (i + 1 < argc) {
                    key_file = argv[++i];
                    has_overrides = true;
                }
                command_override = true;
                has_overrides = true;
            } else if (arg == "-h" || arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -p, --port PORT      Port to listen on\n"
                          << "  -f, --fork           Enable multi-client support with fork\n"
                          << "  -c, --command        Enable command execution mode\n"
                          << "  --reconfigure        Re-run setup wizard\n"
                          << "  -h, --help           Show this help message\n";
                return 0;
            }
        }
        
        // Load or create configuration
        Config config;
        
        // Run setup wizard if needed
        if (reconfigure || config.isFirstRun()) {
            printBanner();
            
            auto auth = std::make_shared<Auth>();
            SetupWizard wizard(config, auth);
            
            if (!wizard.run()) {
                std::cerr << Color::ROSE << "Setup failed" << Color::RESET << std::endl;
                return 1;
            }
            // Wizard completed, config is now in memory
        } else {
            // Load existing config
            if (!config.load()) {
                std::cerr << Color::ROSE << "Failed to load configuration" << Color::RESET << std::endl;
                return 1;
            }
        }
        
        // Display banner
        if (!reconfigure && !config.isFirstRun()) {
            printBanner();
        }
        
        // Get configuration values (command-line overrides config file)
        int port = port_override > 0 ? port_override : config.getInt("port", 8080);
        bool use_fork = fork_override || (!has_overrides && config.getBool("use_fork", false));
        bool command_mode = command_override || (!has_overrides && config.getBool("command_mode", false));
        
        // Restart loop
        bool should_restart = true;
        while (should_restart) {
            // Create server
            Server server(port);
            g_server = &server;
            
            // Register signal handlers
            signal(SIGINT, signalHandler);
            signal(SIGTERM, signalHandler);
            
            // Configure server
            
            // Enable TLS if requested
            if (tls_override) {
                if (cert_file.empty() || key_file.empty()) {
                    std::cerr << Color::ROSE << "TLS enabled but certificate/key files not specified" << Color::RESET << std::endl;
                    return 1;
                }
                server.enableTLS(cert_file, key_file);
            }
            server.setUseFork(use_fork);
            server.setCommandMode(command_mode);
            
            // Start and run server
            server.start();
            server.run();
            
            // Check if restart was requested
            if (server.isRestartRequested()) {
                std::cout << Color::THEME << "\n=== Restarting server ===\n" << Color::RESET << std::endl;
                // Small delay before restart
                sleep(1);
            } else {
                should_restart = false;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << Color::ROSE << "Fatal error: " << e.what() << Color::RESET << std::endl;
        return 1;
    }
    
    return 0;
}
