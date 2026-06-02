#include "server.hpp"

Server::Server(int port) : port_(port), server_id_(-1) {};

Server::~Server() {
  if (server_id_ != -1) {
    close(server_id_);
  }
}

bool Server::Start() {
  server_id_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_id_ < 0) {
    std::cerr << "Error: failed to create socket" << "\n";
    return false;
  }
  int opt = 1;
  setsockopt(server_id_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_);

  if (bind(server_id_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
    std::cerr << "Error: failed to bind to port " << port_ << "\n";
    return false;
  }

  if (listen(server_id_, kMaxUsersInQueue) < 0) {
    std::cerr << "Error: failed to listen on socket" << "\n";
    return false;
  }

  std::cout << "Valkey-lite is running on port " << port_ << ". Waiting for connections...\n";
  return true;
}

void Server::Run(Executor& executor) {
  sockaddr_in address;

  while (true) {
    socklen_t addrlen = sizeof(address);
    int client_id = accept(server_id_, reinterpret_cast<sockaddr*>(&address), &addrlen);
    if (client_id < 0) {
      std::cerr << "Error: accept failed" << "\n";
      continue;
    }
    std::cout << "Client connected" << "\n";
    HandleClient(client_id, executor);

    close(client_id);
    std::cout << "Client session ended. Waiting for a new client..." << "\n";
  }
}

void Server::HandleClient(int client_id, Executor& executor) {
  while (true) {
    std::string buffer(kBufferSize, '\0');
    ssize_t bytes_received = recv(client_id, &buffer[0], buffer.size(), 0);

    if (bytes_received < 0) {
      std::cerr << "Error: failed to read data" << "\n";
      break;
    }

    if (bytes_received == 0) {
      std::cerr << "Error: client disconnected" << "\n";
      break;
    }

    buffer.resize(bytes_received); 
    std::istringstream iss(buffer);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
      tokens.push_back(token);
    }
    if (tokens.empty()) continue;

    if (tokens[0] == "EXIT" || tokens[0] == "exit") {
      std::cout << "Client requested exit" << "\n";
      break;
    }

    for (char& c : tokens[0]) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    std::string result = executor.Execute(tokens);
    if (!result.empty()) {
      result += "\n";
      send(client_id, result.data(), result.size(), 0);
    }
  }
}