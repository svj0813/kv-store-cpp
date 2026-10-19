#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include "wal.hpp"
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <optional>

class KVStore {
private:
    std::unordered_map<std::string, std::string> store_;
    mutable std::shared_mutex mutex_;
    WriteAheadLog wal_;

    void replay_log();

public:
    explicit KVStore(const std::string& wal_path = "database.log");
    ~KVStore() = default;

    // Core Engine Operations
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);

    // Binary RDB Snapshot Persistence
    bool save_rdb(const std::string& filepath = "dump.rdb") const;
    bool load_rdb(const std::string& filepath = "dump.rdb");

    size_t size() const;
};

#endif // KV_STORE_HPP