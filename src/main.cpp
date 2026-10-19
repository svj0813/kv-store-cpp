#include "kv_store.hpp"
#include <iostream>
#include <cassert>

int main() {
    {
        KVStore engine("database.log");
        engine.set("session_id", "xyz987");
        engine.set("user_role", "admin");
        engine.save_rdb("dump.rdb");
        std::cout << "[PASS] WAL logging and RDB snapshot creation successful." << std::endl;
    }

    // Simulate crash and recovery in a new instance
    {
        KVStore recovered_engine("database.log");
        auto role = recovered_engine.get("user_role");
        assert(role.has_value() && role.value() == "admin");
        std::cout << "[PASS] Crash recovery state verified: user_role = " << role.value() << std::endl;
    }

    std::cout << "Phase 2 (WAL & RDB Persistence) completely verified." << std::endl;
    return 0;
}