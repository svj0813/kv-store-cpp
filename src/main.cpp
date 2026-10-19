#include "server.hpp"

int main() {
    Server server(6379, "database.log");
    server.start();
    return 0;
}