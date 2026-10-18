#include "kv_store.hpp"
#include <mutex>
#include <iostream>

KVStore::KVStore(WAL& wal_instance) : wal(wal_instance) {
    loadSnapshot("dump.rdb");
    ttl_thread = std::thread(&KVStore::startTTLWorker, this);
}

KVStore::~KVStore() {
    running = false;
    if (ttl_thread.joinable()) {
        ttl_thread.join();
    }
}

void KVStore::set(const std::string& key, const std::string& value, int ttl_seconds) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    CacheValue cv;
    cv.value = value;
    if (ttl_seconds > 0) {
        cv.expiry = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    }
    store[key] = cv;
    wal.logSet(key, value);
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    auto it = store.find(key);
    if (it == store.end()) {
        return std::nullopt;
    }

    if (it->second.expiry.has_value() && std::chrono::steady_clock::now() >= it->second.expiry.value()) {
        lock.unlock();
        del(key);
        return std::nullopt;
    }

    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    bool erased = store.erase(key) > 0;
    if (erased) {
        wal.logDel(key);
    }
    return erased;
}

bool KVStore::expire(const std::string& key, int seconds) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    auto it = store.find(key);
    if (it == store.end()) {
        return false;
    }
    it->second.expiry = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    return true;
}

void KVStore::startTTLWorker() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto now = std::chrono::steady_clock::now();

        std::unique_lock<std::shared_mutex> lock(rw_mutex);
        for (auto it = store.begin(); it != store.end(); ) {
            if (it->second.expiry.has_value() && now >= it->second.expiry.value()) {
                wal.logDel(it->first);
                it = store.erase(it);
            } else {
                ++it;
            }
        }
    }
}

// Phase 6 Snapshot Implementations
bool KVStore::saveSnapshot(const std::string& filename) {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return false;

    size_t size = store.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& [key, cv] : store) {
        size_t k_len = key.length();
        size_t v_len = cv.value.length();

        file.write(reinterpret_cast<const char*>(&k_len), sizeof(k_len));
        file.write(key.data(), k_len);
        file.write(reinterpret_cast<const char*>(&v_len), sizeof(v_len));
        file.write(cv.value.data(), v_len);
    }

    file.close();
    wal.clear(); 
    return true;
}

bool KVStore::loadSnapshot(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    size_t size = 0;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));

    for (size_t i = 0; i < size; ++i) {
        size_t k_len = 0, v_len = 0;
        file.read(reinterpret_cast<char*>(&k_len), sizeof(k_len));
        std::string key(k_len, '\0');
        file.read(&key[0], k_len);

        file.read(reinterpret_cast<char*>(&v_len), sizeof(v_len));
        std::string val(v_len, '\0');
        file.read(&val[0], v_len);

        CacheValue cv;
        cv.value = val;
        store[key] = cv;
    }

    file.close();
    std::cout << "[Database] Loaded " << size << " keys from snapshot (" << filename << ").\n";
    return true;
}

void KVStore::bgsave() {
    std::thread bg_thread([this]() {
        if (saveSnapshot("dump.rdb")) {
            std::cout << "[Background Task] Background snapshot saved successfully.\n";
        }
    });
    bg_thread.detach();
}