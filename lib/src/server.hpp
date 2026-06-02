#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h>     
#include "executor.hpp"

inline const int kValkeyServerBasePort = 6379;
inline const int kMaxUsersInQueue = 5;
inline const int kBufferSize = 1024;

class Server {
  int port_;
  int server_id_;
public:
  Server(int port = kValkeyServerBasePort);

  ~Server();

  bool Start();

  void Run(Executor& executor);

private:
  void HandleClient(int client_id, Executor& executor);
};