# kv-store-cpp 💾

So i built a multi-threaded key-value store from scratch in c++17 because i wanted to see how databases actually work under the hood instead of just watching tutorials lol. 

it's basically a mini redis clone that handles concurrent requests, doesn't lose data when it crashes, and runs inside docker. pretty cool for a first sem student if i do say so myself 

---

## what it actually does:
- **multithreading:** uses `std::shared_mutex` so multiple people can read stuff at the same time without lagging out.
- **crash recovery (wal):** writes operations to a log (`database.log`) *before* changing memory, so if the server blows up, it recovers everything on restart.
- **snapshots (rdb):** type `SAVE` and it dumps everything into a compact binary `.rdb` file.
- **tcp server:** has a custom non-blocking socket server listening on port `6379` so you can talk to it using netcat (`nc`) just like real redis.

---

## tech stack 🛠️
- c++17
- cmake
- linux system calls (sockets, pthreads)
- docker & docker-compose

---

future plans 🚀

might turn this into a distributed raft cluster or build an lsm-tree storage engine later. leave a star if you think it's neat! ⭐

## how to run it locally 🏃‍♂️***********

clone the repo and build it with cmake:

```bash
git clone [https://github.com/svj0813/kv-store-cpp.git](https://github.com/svj0813/kv-store-cpp.git)
cd kv-store-cpp

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4

./build/kvstore



##running with docker 🐳

if you don't want to compile manually:
Bash

sudo docker compose up --build -d



##testing it out with netcat:

open another terminal window and run these commands:
Bash

# test ping
echo -e "PING\n" | nc localhost 6379

# set a key
echo -e "SET my_key hello_world\n" | nc localhost 6379

# get the key back
echo -e "GET my_key\n" | nc localhost 6379

# save a snapshot to disk
echo -e "SAVE\n" | nc localhost 6379


