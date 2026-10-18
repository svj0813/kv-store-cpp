#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include "wal.hpp"

class KVStore {
private:
    std::unordered_map<std::string, std::string> store;
    WAL& wal;

public:
    // Pass WAL reference in constructor
    KVStore(WAL& wal_instance) : wal(wal_instance) {}

    // Database Operations
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
};