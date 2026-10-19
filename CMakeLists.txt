cmake_minimum_required(VERSION 3.16)
project(KVStoreCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/kv_store.cpp
    src/wal.cpp
    src/server.cpp
    src/main.cpp
)

add_executable(kvstore ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(kvstore PRIVATE Threads::Threads)