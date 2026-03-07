#include "../INC/Server.hpp"

Client *Server::GetClient(int current_fd){
	size_t clients_size = this->clients.size();

	for (size_t i = 0; i < clients_size; i++){
		int client_fd = this->clients[i].GetFd();
        
		if (client_fd == current_fd)
			return &this->clients[i];
	}
	return NULL;
}

bool Server::isClientRegistered(int fd)
{
    Client *client = GetClient(fd);
    if (client == NULL) {
        printStatus(RED, "in Registration Check", fd, "Client not found");
        return false;
    }
    
    if (client->GetNickName().empty() || client->GetNickName() == "*") {
        printStatus(RED, "in Registration Check", fd, "No nickname set");
        return false;
    }
    
    if (client->GetUserName().empty()) {
        printStatus(RED, "in Registration Check", fd, "No username set");
        return false;
    }
    
    if (!client->GetLogedIn()) {
        printStatus(RED, "in Registration Check", fd, "Not logged in");
        return false;
    }
    
    return true;
}