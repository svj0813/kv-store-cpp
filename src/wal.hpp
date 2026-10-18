#pragma once
#include <string>
#include <fstream>
#include <iostream>

class WAL {
private:
    std::string log_filename;
    std::ofstream log_file;

public:
    WAL(const std::string& filename) : log_filename(filename) {
        log_file.open(log_filename, std::ios::app);
    }

    ~WAL() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    void logSet(const std::string& key, const std::string& value) {
        if (log_file.is_open()) {
            log_file << "SET " << key << " " << value << "\n";
            log_file.flush();
        }
    }

    void logDel(const std::string& key) {
        if (log_file.is_open()) {
            log_file << "DEL " << key << "\n";
            log_file.flush();
        }
    }

    void clear() {
        if (log_file.is_open()) {
            log_file.close();
        }
        log_file.open(log_filename, std::ios::out | std::ios::trunc);
        log_file.close();
        log_file.open(log_filename, std::ios::app);
    }
};