#include <iostream>
#include "kv_store.hpp"

int main() {
    KVStore db;

    db.set("user:101", "Shaurya");
    db.set("role", "Developer");

    auto user = db.get("user:101");
    if (user) {
        std::cout << "[GET] user:101 => " << *user << std::endl;
    }

    if (db.del("role")) {
        std::cout << "[DEL] Key 'role' deleted successfully." << std::endl;
    }

    auto deletedRole = db.get("role");
    if (!deletedRole) {
        std::cout << "[GET] Key 'role' not found (as expected)." << std::endl;
    }

    return 0;
}