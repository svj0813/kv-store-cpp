#include "server.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>

Server::Server(int port, const std::string& wal_path)
    : port_(port), server_fd_(-1), store_(wal_path) {}

Server::~Server() {
    if (server_fd_ != -1) {
        close(server_fd_);
    }
}

std::string Server::process_command(const std::string& raw_cmd) {
    std::stringstream ss(raw_cmd);
    std::string cmd, key, val;
    ss >> cmd;

    if (cmd == "PING") {
        return "PONG\n";
    } else if (cmd == "SET") {
        ss >> key >> val;
        if (key.empty() || val.empty()) return "ERR syntax error\n";
        store_.set(key, val);
        return "OK\n";
    } else if (cmd == "GET") {
        ss >> key;
        if (key.empty()) return "ERR syntax error\n";
        auto res = store_.get(key);
        if (res.has_value()) {
            return res.value() + "\n";
        }
        return "(nil)\n";
    } else if (cmd == "DEL") {
        ss >> key;
        if (key.empty()) return "ERR syntax error\n";
        bool erased = store_.del(key);
        return erased ? ":1\n" : ":0\n";
    } else if (cmd == "SAVE") {
        bool saved = store_.save_rdb("dump.rdb");
        return saved ? "OK\n" : "ERR save failed\n";
    }

    return "ERR unknown command\n";
}

void Server::handle_client(int client_socket) {
    char buffer[1024] = {0};
    while (true) {
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) break;

        buffer[bytes_read] = '\0';
        std::string response = process_command(std::string(buffer));
        send(client_socket, response.c_str(), response.size(), 0);
    }
    close(client_socket);
}

void Server::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port_ << "\n";
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        std::cerr << "Listen failed\n";
        return;
    }

    std::cout << "[KVStore Server] Listening on 0.0.0.0:" << port_ << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t addrlen = sizeof(client_addr);
        int client_socket = accept(server_fd_, (struct sockaddr*)&client_addr, &addrlen);
        if (client_socket < 0) continue;

        // Spawn a thread per client connection
        std::thread(&Server::handle_client, this, client_socket).detach();
    }
}