#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "kv_store.hpp"

class Server {
private:
    int server_fd;
    int port;
    KVStore& db;

    // Helper to process raw command strings from clients
    std::string processCommand(const std::string& raw_cmd) {
        std::stringstream ss(raw_cmd);
        std::string action, key, value;
        ss >> action;

        if (action == "SET") {
            ss >> key >> value;
            if (key.empty() || value.empty()) {
                return "ERROR: Usage SET <key> <value>\n";
            }
            db.set(key, value);
            return "OK\n";
        } 
        else if (action == "GET") {
            ss >> key;
            if (key.empty()) {
                return "ERROR: Usage GET <key>\n";
            }
            auto res = db.get(key);
            if (res) {
                return *res + "\n";
            }
            return "(nil)\n";
        } 
        else if (action == "DEL") {
            ss >> key;
            if (key.empty()) {
                return "ERROR: Usage DEL <key>\n";
            }
            bool deleted = db.del(key);
            return deleted ? "1\n" : "0\n";
        } 
        else if (action == "PING") {
            return "PONG\n";
        }

        return "ERROR: Unknown command\n";
    }

public:
    Server(int port_num, KVStore& store) : port(port_num), db(store), server_fd(-1) {}

    ~Server() {
        if (server_fd != -1) {
            close(server_fd);
        }
    }

    void start() {
        // 1. Create IPv4 TCP socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            std::cerr << "[Server Error] Socket creation failed\n";
            return;
        }

        // Allow address reuse (prevents "Address already in use" errors)
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // 2. Bind socket to IP address and port
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "[Server Error] Bind failed on port " << port << "\n";
            return;
        }

        // 3. Listen for incoming connections
        if (listen(server_fd, 5) < 0) {
            std::cerr << "[Server Error] Listen failed\n";
            return;
        }

        std::cout << "[Server] Database server listening on port " << port << "...\n";

        // 4. Accept client connections in a loop
        while (true) {
            int addrlen = sizeof(address);
            int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            if (client_fd < 0) {
                std::cerr << "[Server Error] Accept connection failed\n";
                continue;
            }

            std::cout << "[Server] Client connected!\n";

            char buffer[1024] = {0};
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
                
                if (bytes_read <= 0) {
                    std::cout << "[Server] Client disconnected.\n";
                    break;
                }

                std::string request(buffer);
                std::string response = processCommand(request);
                send(client_fd, response.c_str(), response.length(), 0);
            }

            close(client_fd);
        }
    }
};