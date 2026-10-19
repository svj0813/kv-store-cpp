#include "kv_store.hpp"
#include <fstream>
#include <sstream>
#include <mutex>

KVStore::KVStore(const std::string& wal_path) : wal_(wal_path) {
    load_rdb("dump.rdb"); // Load snapshot if available
    replay_log();          // Replay un-checkpointed WAL events
}

void KVStore::replay_log() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    std::ifstream file("database.log");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cmd, key, value;
        ss >> cmd >> key;

        if (cmd == "SET") {
            ss >> value;
            store_[key] = value;
        } else if (cmd == "DEL") {
            store_.erase(key);
        }
    }
}

void KVStore::set(const std::string& key, const std::string& value) {
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        store_[key] = value;
    }
    wal_.log_set(key, value);
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it != store_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool KVStore::del(const std::string& key) {
    bool erased = false;
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        erased = (store_.erase(key) > 0);
    }
    if (erased) {
        wal_.log_del(key);
    }
    return erased;
}

bool KVStore::save_rdb(const std::string& filepath) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    size_t count = store_.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& [key, value] : store_) {
        size_t klen = key.size();
        size_t vlen = value.size();

        file.write(reinterpret_cast<const char*>(&klen), sizeof(klen));
        file.write(key.data(), klen);

        file.write(reinterpret_cast<const char*>(&vlen), sizeof(vlen));
        file.write(value.data(), vlen);
    }

    return true;
}

bool KVStore::load_rdb(const std::string& filepath) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    size_t count = 0;
    if (!file.read(reinterpret_cast<char*>(&count), sizeof(count))) return false;

    store_.clear();
    for (size_t i = 0; i < count; ++i) {
        size_t klen = 0, vlen = 0;
        
        file.read(reinterpret_cast<char*>(&klen), sizeof(klen));
        std::string key(klen, '\0');
        file.read(&key[0], klen);

        file.read(reinterpret_cast<char*>(&vlen), sizeof(vlen));
        std::string value(vlen, '\0');
        file.read(&value[0], vlen);

        store_[key] = value;
    }

    return true;
}

size_t KVStore::size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return store_.size();
}