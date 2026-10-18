#include <iostream>
#include "wal.hpp"
#include "kv_store.hpp"

int main() {
    // Initialize Write-Ahead Log storing to 'database.log'
    WAL wal("database.log");
    
    // Initialize Database with WAL attached
    KVStore db(wal);

    std::cout << "--- Writing Operations ---" << std::endl;
    db.set("user:101", "Shaurya");
    db.set("role", "Developer");
    db.set("status", "Active");

    auto user = db.get("user:101");
    if (user) {
        std::cout << "[GET] user:101 => " << *user << std::endl;
    }

    if (db.del("role")) {
        std::cout << "[DEL] Key 'role' deleted successfully." << std::endl;
    }

    return 0;
}