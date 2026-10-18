#include "kv_store.hpp"

void KVStore::set(const std::string& key, const std::string& value) {
    store[key] = value;
}

std::optional<std::string> KVStore::get(const std::string& key) {
    auto it = store.find(key);
    if (it != store.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool KVStore::del(const std::string& key) {
    return store.erase(key) > 0;
}