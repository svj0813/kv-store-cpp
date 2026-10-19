#ifndef SERVER_HPP
#define SERVER_HPP

#include "kv_store.hpp"
#include <string>
#include <netinet/in.h>

class Server {
private:
    int port_;
    int server_fd_;
    KVStore store_;

    void handle_client(int client_socket);
    std::string process_command(const std::string& raw_cmd);

public:
    explicit Server(int port = 6379, const std::string& wal_path = "database.log");
    ~Server();

    void start();
};

#endif // SERVER_HPP