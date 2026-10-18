# High-Performance Multi-Threaded C++ Key-Value Database Engine

A lightweight, multi-threaded, persistent in-memory Key-Value store built from scratch in modern C++17. Designed with POSIX TCP networking, thread-safe read/write concurrency via `std::shared_mutex`, Write-Ahead Logging (WAL), TTL key expiration, and background RDB snapshot persistence.

---

## Key Features & Architecture

```text
                    +--------------------------------+
                    |       TCP Sockets (Client)     |
                    +--------------------------------+
                                    |
                                    v
                    +--------------------------------+
                    |    POSIX Thread Handler        |
                    +--------------------------------+
                                    |
                                    v
                  +------------------------------------+
                  |   std::shared_mutex (RW Lock)     |
                  +------------------------------------+
                        /                        \
                       v                          v
      +------------------------+      +------------------------+
      | In-Memory Hash Map     |      | Write-Ahead Log (WAL)   |
      | (std::unordered_map)   |      | (database.log)         |
      +------------------------+      +------------------------+
                  |                                |
                  v                                v
      +------------------------+      +------------------------+
      | Background TTL Worker  |      | RDB Point-in-Time      |
      | (Active Expiration)    |      | Snapshot (dump.rdb)    |
      +------------------------+      +------------------------+
