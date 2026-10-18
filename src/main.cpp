#include <iostream>
#include "wal.hpp"
#include "kv_store.hpp"
#include "server.hpp"

int main() {
    // 1. Initialize Write-Ahead Log
    WAL wal("database.log");
    
    // 2. Initialize Key-Value Store with WAL attached
    KVStore db(wal);

    // 3. Start TCP Socket Server on Port 6379
    Server server(6379, db);
    
    std::cout << "Starting C++ Key-Value Database Server...\n";
    server.start();

    return 0;
}
