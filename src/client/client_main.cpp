#include "Client.h"
#include "Colors.h"
#include <iostream>
#include <cstdlib>

// ASCII Banner
void printBanner() {
    std::cout << Color::PURPLE << Color::BOLD;
    std::cout << "\n";
    std::cout << "  ███████╗ █████╗ ███████╗██╗   ██╗    ██████╗ ███████╗██╗  ██╗\n";
    std::cout << "  ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝    ██╔══██╗██╔════╝██║  ██║\n";
    std::cout << "  █████╗  ███████║███████╗ ╚████╔╝     ██████╔╝███████╗███████║\n";
    std::cout << "  ██╔══╝  ██╔══██║╚════██║  ╚██╔╝      ██╔══██╗╚════██║██╔══██║\n";
    std::cout << "  ███████╗██║  ██║███████║   ██║       ██║  ██║███████║██║  ██║\n";
    std::cout << "  ╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝       ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n";
    std::cout << Color::RESET;
    std::cout << Color::GRAY << "                    Easy Remote Shell Client v1.0" << Color::RESET << "\n\n";
}

int main(int argc, char* argv[]) {
    try {
        
        std::string host = "127.0.0.1";
        bool use_tls = false;
        int port = 8080;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            if (arg == "-h" || arg == "--host") {
                if (i + 1 < argc) {
                    host = argv[++i];
                }
            } else if (arg == "-p" || arg == "--port") {
                if (i + 1 < argc) {
                    port = std::atoi(argv[++i]);
                }
            } else if (arg == "--help") {
            } else if (arg == "--tls") {
                use_tls = true;
            }
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  -h, --host HOST     Server host (default: 127.0.0.1)\n"
                          << "  -p, --port PORT     Server port (default: 8080)\n"
                          << "  --help              Show this help message\n";
                return 0;
            } else if (i == 1 && arg.find('-') != 0) {
                
                host = arg;
            } else if (i == 2 && arg.find('-') != 0) {
                
                port = std::atoi(arg.c_str());
            }
        }
        
        // Display banner
        printBanner();
        
        // Create client
        
        // Enable TLS if requested
        if (use_tls) {
            client.enableTLS();
        }
        Client client(host, port);
        
        // Connect to server
        if (!client.connect()) {
            std::cerr << Color::ROSE << "Failed to connect to server." << Color::RESET << std::endl;
            return 1;
        }
        
        
        client.runInteractiveShell();
        
    } catch (const std::exception& e) {
        std::cerr << Color::ROSE << "Error: " << e.what() << Color::RESET << std::endl;
        return 1;
    }
    
    return 0;
}
