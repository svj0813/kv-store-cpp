#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <chrono>
#include <atomic>
#include <thread>
#include "wal.hpp"

struct CacheValue {
    std::string value;
    std::optional<std::chrono::steady_clock::time_point> expiry;
};

class KVStore {
private:
    std::unordered_map<std::string, CacheValue> store;
    WAL& wal;
    mutable std::shared_mutex rw_mutex;

    // Background TTL cleanup thread controls
    std::atomic<bool> running{true};
    std::thread ttl_thread;

    void startTTLWorker();

public:
    KVStore(WAL& wal_instance);
    ~KVStore();

    // Database Operations with optional TTL (seconds)
    void set(const std::string& key, const std::string& value, int ttl_seconds = -1);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool expire(const std::string& key, int seconds);
};