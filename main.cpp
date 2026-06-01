#include <iostream>
#include <sstream>
#include "executor.hpp"
#include "server.hpp"
#include <vector>
#include <string>
#include <cctype>

const int kMaxArgc = 3;
const int kMinArgc = 1;
const int kMaxMemoryToken = 2;

int main(int argc, const char** argv) {
    if (argc != kMaxArgc && argc != kMinArgc) {
        std::cerr << "Wrong amount of arguments" << "\n";
        return 1;
    }
    std::string maxmemory_limit = "0";
    if (argc == kMaxArgc) {
        if (std::string(argv[1]) != "--maxmemory") {
            std::cerr << "Unknown argument: " << argv[1] << "\n";
            std::cerr << "Usage: ./valkey [--maxmemory <bytes>]\n";
            return 1;
        }
        maxmemory_limit = argv[kMaxMemoryToken];
    }
    Executor executor(maxmemory_limit);
    Server server(6379);

    if (!server.Start()) {
        return 1;
    }

    server.Run(executor);

    return 0;
}