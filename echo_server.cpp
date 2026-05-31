#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h>     

int main() {
  int server_id = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(6379);

  if (bind(server_id, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
    std::cerr << "Error: bind failed" << "\n";
    close(server_id);
    return 1;
  }

  if (listen(server_id, 5) < 0) {
    std::cerr << "Error: listen failed" << "\n";
    close(server_id);
    return 1;
  }

  socklen_t addrlen = sizeof(address);
  int client_id = accept(server_id, reinterpret_cast<sockaddr*>(&address), &addrlen);
  if (client_id < 0) {
    std::cerr << "Error: accept failed" << "\n";
    close(server_id);
    return 1;
  }

  std::string buffer(1024, '\0');

  size_t bytes_received = recv(client_id, &buffer[0], buffer.size(), 0);
  if (bytes_received < 0) {
    std::cerr << "Error: failed reading data" << "\n";
  } else if (bytes_received == 0) {
    std::cout << "Client closed connection" << "\n";
  } else {
    buffer.resize(bytes_received); 
    std::cout << "Client sent: " << buffer;

    send(client_id, buffer.data(), buffer.size(), 0);
  }

  close(client_id);
  close(server_id);
  return 0;
}
