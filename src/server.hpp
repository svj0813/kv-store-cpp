#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "kv_store.hpp"

class Server {
private:
    int server_fd;
    int port;
    KVStore& db;

    void handleClient(int client_fd) {
        char buffer[1024] = {0};
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
            
            if (bytes_read <= 0) {
                break;
            }

            std::string request(buffer);
            std::string response = processCommand(request);
            send(client_fd, response.c_str(), response.length(), 0);
        }

        close(client_fd);
    }

    std::string processCommand(const std::string& raw_cmd) {
        std::stringstream ss(raw_cmd);
        std::string action, key, value, option;
        ss >> action;

        if (action == "SET") {
            ss >> key >> value;
            if (key.empty() || value.empty()) {
                return "ERROR: Usage SET <key> <value> [EX seconds]\n";
            }
            
            int ttl = -1;
            if (ss >> option && option == "EX") {
                ss >> ttl;
            }
            
            db.set(key, value, ttl);
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
        else if (action == "EXPIRE") {
            int seconds;
            ss >> key >> seconds;
            if (key.empty()) {
                return "ERROR: Usage EXPIRE <key> <seconds>\n";
            }
            bool set_exp = db.expire(key, seconds);
            return set_exp ? "1\n" : "0\n";
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
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            std::cerr << "[Server Error] Socket creation failed\n";
            return;
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "[Server Error] Bind failed on port " << port << "\n";
            return;
        }

        if (listen(server_fd, 10) < 0) {
            std::cerr << "[Server Error] Listen failed\n";
            return;
        }

        std::cout << "[Server] Multi-Threaded Database server listening on port " << port << "...\n";

        while (true) {
            int addrlen = sizeof(address);
            int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            if (client_fd < 0) {
                continue;
            }

            std::thread client_thread(&Server::handleClient, this, client_fd);
            client_thread.detach();
        }
    }
};