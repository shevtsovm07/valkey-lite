#include <iostream>
#include <sstream>
#include "executor.hpp"
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
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line == "EXIT" || line == "exit") break;

        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (iss >> token) {
            tokens.push_back(token);
        }
        if (tokens.empty()) continue;

        for (char& c : tokens[0]) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        std::string result = executor.Execute(tokens);
        if (!result.empty()) std::cout << result << "\n";
    }
}