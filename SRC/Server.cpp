#include "../INC/Server.hpp"

bool Server::isBotfull = false;
Server::Server(){this->server_socket_fd = -1;}
Server::~Server(){}
Server::Server(Server const &src){*this = src;}
Server &Server::operator=(Server const &src){
	if (this != &src){
		/*
		struct sockaddr_in add;
		struct sockaddr_in cliadd;
		struct pollfd new_cli;
		*/
		this->port = src.port;
		this->server_socket_fd = src.server_socket_fd;
		this->password = src.password;
		this->clients = src.clients;
		this->channels = src.channels;
		this->fds = src.fds;
		this->isBotfull = src.isBotfull;
	}
	return *this;
}



Client *Server::GetClientNick(std::string nickname){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].GetNickName() == nickname)
			return &this->clients[i];
	}
	return NULL;
}

Channel *Server::GetChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			return &channels[i];
	}
	return NULL;
}
//---------------//Getters
//---------------//Setters
void Server::SetFd(int fd){this->server_socket_fd = fd;}
void Server::SetPort(int port){this->port = port;}
void Server::SetPassword(std::string password){this->password = password;}
std::string Server::GetPassword(){return this->password;}
void Server::AddClient(Client newClient){this->clients.push_back(newClient);}
void Server::AddChannel(Channel newChannel){this->channels.push_back(newChannel);}
void Server::AddFds(pollfd newFd){this->fds.push_back(newFd);}
//---------------//Setters
//---------------//Remove Methods

//This method is responsible for removing a client from the server entirely.
//It iterates through the clients vector of the Server object.
//if is found, it removes the client from the server's global list of clients.

void Server::RemoveClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].GetFd() == fd)
			{this->clients.erase(this->clients.begin() + i); return;}
	}
}
void Server::RemoveChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			{this->channels.erase(this->channels.begin() + i); return;}
	}
}
void Server::RemoveFds(int fd){
	for (size_t i = 0; i < this->fds.size(); i++){
		if (this->fds[i].fd == fd)
			{this->fds.erase(this->fds.begin() + i); return;}
	}
}
void	Server::RmChannels(int fd){
	//go through all channels and remove the client from each channel
	for (size_t i = 0; i < this->channels.size(); i++){
		int flag = 0;

		//check if the client is in the channel as a client or admin
		if (channels[i].get_client(fd)){
			channels[i].remove_client(fd);
			flag = 1;
		}

		//check if the client is in the channel as an admin
		else if (channels[i].get_admin(fd)){
			channels[i].remove_admin(fd);
			flag = 1;
		}

		//if the channel is empty, delete the channel
		if (channels[i].GetClientsNumber() == 0){
			channels.erase(channels.begin() + i);
			//the channel is erased and the other elements are shifted left 
			//so we need to decrement i because the next element is now at the current index instead of i+1
			//like the channel[3] is erased and now the channel[4] is at channel[3] position so we need to decrement i to check the new channel[3]
			i--; 
			continue;
		}
		if (flag){
			//Constructs a QUIT message in the IRC protocol format 
			std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT Quit\r\n";
			//notify all clients in the channel that the client has left
			channels[i].sendTo_all(rpl);
		}
	}
}
//---------------//Remove Methods
//---------------//Send Methods
void Server::senderror(int code, std::string clientname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << " " << channelname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::_sendResponse(std::string response, int fd)
{
	//send() sends the response string to the client identified by the file descriptor fd.
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}
//---------------//Send Methods
//---------------//Close and Signal Methods
bool Server::Signal = false;

//The handler must have this signature:
// void handler(int signal);

/*
If the signal interrupts code that was already using std::cout, this can cause:
Deadlocks
Undefined behavior
*/
void Server::SignalHandler(int signum)
{
	(void)signum;
	const char* msg = "\nSignal Received!\n";
    write(STDOUT_FILENO, msg, 18);
	Server::Signal = true;
}

void	Server::close_fds(){
	for(size_t i = 0; i < clients.size(); i++){
		printStatus(RED, "Client", clients[i].GetFd(), "Disconnected");
		close(clients[i].GetFd());
	}
	if (server_socket_fd != -1){
		printStatus(RED, "Server", server_socket_fd, "Disconnected");
		close(server_socket_fd);
	}
}

