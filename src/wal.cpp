#include "wal.hpp"

WriteAheadLog::WriteAheadLog(const std::string& filepath) : filepath_(filepath) {
    log_file_.open(filepath_, std::ios::app);
}

WriteAheadLog::~WriteAheadLog() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void WriteAheadLog::log_set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_ << "SET " << key << " " << value << "\n";
        log_file_.flush();
    }
}

void WriteAheadLog::log_del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_ << "DEL " << key << "\n";
        log_file_.flush();
    }
}

void WriteAheadLog::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
    log_file_.open(filepath_, std::ios::trunc);
}