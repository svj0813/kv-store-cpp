# kv-store-cpp 🚀

i made a redis clone from scratch in c++ because i was bored and wanted to build something fast. handles like 17k requests a second on my fedora setup which is kinda crazy lol.

built with raw posix TCP sockets, `std::shared_mutex` for multi-threading, WAL logging so stuff doesn't wipe when it crashes, and background snapshots so the log file doesn't get huge.

## stuff it can do
- **Fast Multithreading**: uses `std::shared_mutex` so multiple clients can read at the same time without blocking each other.
- **WAL Logging**: every `SET` or `DEL` gets appended to `database.log` so you don't lose data if you ctrl+c.
- **Key Expiration (TTL)**: set keys to expire after X seconds. a background thread sweeps and deletes dead keys every second.
- **RDB Snapshots**: type `SAVE` or `BGSAVE` to dump the whole memory map into a binary `dump.rdb` file and wipe the log clean.
- **Benchmarker**: built a multi-threaded stress tester tool to throw thousands of requests at it.

---

architecture (how it works basically) 
## commands supported

```text
SET key value         -> saves a key
SET key value EX 5    -> saves key, expires in 5 seconds
GET key               -> gets value (or nil if expired/missing)
DEL key               -> deletes key
EXPIRE key 10         -> sets expiration to 10 secs
SAVE                  -> saves snapshot to dump.rdb synchronously
BGSAVE                -> saves snapshot on a background thread
PING                  -> returns PONG

[nc / TCP client] ──> [POSIX Thread per Client]
                              │
                              ▼
                   [std::shared_mutex Lock]
                        /            \
                       v              v
            [in-memory hashmap]   [database.log WAL]
                     │                    │
                     ▼                    ▼
             [TTL worker thread]  [dump.rdb snapshot]

how to run this on linux

you need cmake and g++ (c++17).

# build everything
cmake -B build
cmake --build build

# start the database server
./build/kvstore

in another terminal window, connect using netcat:

nc localhost 6379

or run the benchmark tool to flex on your CPU:

./build/kvbench

benchmark results

tested on my fedora machine with 4 threads and 10,000 requests:

  -  Throughput: ~17,036 Requests/Sec

  -  Success rate: 10000 / 10000
