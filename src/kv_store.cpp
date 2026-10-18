#include "kv_store.hpp"

void KVStore::set(const std::string& key, const std::string& value) {
    store[key] = value;
    wal.logSet(key, value); // Log to disk
}

std::optional<std::string> KVStore::get(const std::string& key) {
    auto it = store.find(key);
    if (it != store.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool KVStore::del(const std::string& key) {
    bool erased = store.erase(key) > 0;
    if (erased) {
        wal.logDel(key); // Log to disk
    }
    return erased;
}