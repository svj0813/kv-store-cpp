cmake_minimum_required(VERSION 3.10)
project(KVStoreCPP VERSION 1.0)

# Enforce Modern C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/kv_store.cpp
)

# Build executable
add_executable(kvstore ${SOURCES})