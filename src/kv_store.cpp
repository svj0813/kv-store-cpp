#include "kv_store.hpp"
#include <mutex>

KVStore::KVStore(WAL& wal_instance) : wal(wal_instance) {
    // Spawn background thread to sweep expired keys
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

    // Passive Expiration Check
    if (it->second.expiry.has_value() && std::chrono::steady_clock::now() >= it->second.expiry.value()) {
        lock.unlock(); // Release read lock to acquire write lock for deletion
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
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Sweep every 1 second
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
