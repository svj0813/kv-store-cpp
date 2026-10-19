#ifndef WAL_HPP
#define WAL_HPP

#include <string>
#include <fstream>
#include <mutex>

class WriteAheadLog {
private:
    std::string filepath_;
    std::ofstream log_file_;
    std::mutex mutex_;

public:
    explicit WriteAheadLog(const std::string& filepath = "database.log");
    ~WriteAheadLog();

    void log_set(const std::string& key, const std::string& value);
    void log_del(const std::string& key);
    void clear();
};

#endif // WAL_HPP