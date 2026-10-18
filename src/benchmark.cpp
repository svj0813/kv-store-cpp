#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

constexpr int NUM_THREADS = 4;
constexpr int REQUESTS_PER_THREAD = 2500; // Total 10,000 requests
std::atomic<int> successful_requests(0);

void runBenchmark(int thread_id) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6379);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return;
    }

    char buffer[128];
    for (int i = 0; i < REQUESTS_PER_THREAD; ++i) {
        std::string cmd = "SET bench_key_" + std::to_string(thread_id) + "_" + std::to_string(i) + " bench_val\n";
        send(sock, cmd.c_str(), cmd.length(), 0);

        memset(buffer, 0, sizeof(buffer));
        int bytes = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            successful_requests++;
        }
    }

    close(sock);
}

int main() {
    std::cout << "Starting Multi-Threaded Database Benchmark...\n";
    std::cout << "Threads: " << NUM_THREADS << " | Total Requests: " << (NUM_THREADS * REQUESTS_PER_THREAD) << "\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(runBenchmark, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "-------------------------------------------\n";
    std::cout << "Benchmark Complete!\n";
    std::cout << "Successful Requests: " << successful_requests << "\n";
    std::cout << "Time Elapsed: " << elapsed.count() << " seconds\n";
    std::cout << "Throughput: " << (successful_requests / elapsed.count()) << " Requests/Sec (RPS)\n";
    std::cout << "-------------------------------------------\n";

    return 0;
}