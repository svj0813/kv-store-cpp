#pragma once
#include <string>
#include <fstream>
#include <iostream>

class WAL {
private:
    std::ofstream logFile;
    std::string filename;

public:
    WAL(const std::string& filepath) : filename(filepath) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "[WAL Error] Failed to open log file: " << filename << std::endl;
        }
    }

    ~WAL() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    // Append a SET operation to the log
    void logSet(const std::string& key, const std::string& value) {
        if (logFile.is_open()) {
            logFile << "SET " << key << " " << value << "\n";
            logFile.flush(); // Ensure data is immediately written to disk
        }
    }

    // Append a DEL operation to the log
    void logDel(const std::string& key) {
        if (logFile.is_open()) {
            logFile << "DEL " << key << "\n";
            logFile.flush();
        }
    }
};