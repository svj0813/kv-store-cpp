cmake_minimum_required(VERSION 3.10)
project(KVStoreEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kvstore 
    src/main.cpp 
    src/kv_store.cpp
)

add_executable(kvbench
    src/benchmark.cpp
)