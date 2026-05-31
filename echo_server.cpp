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
    std::cerr << "Error: failed port 6379 creation" << "\n";
    close(server_id);
    return 1;
  }

  if (listen(server_id, 5) < 0) {
    std::cerr << "Error: failed turning listening on" << "\n";
    close(server_id);
    return 1;
  }

  socklen_t addrlen = sizeof(address);
  int client_id = accept(server_id, reinterpret_cast<sockaddr*>(&address), &addrlen);
  if (client_id < 0) {
    std::cerr << "Error: failed accepting linkage" << "\n";
    close(server_id);
    return 1;
  }
  std::cout << "ОТЛАДКА: КЛИЕНТ ПОДКЛЮЧЕН" << "\n";

  close(client_id);
  close(server_id);
  return 0;
}
