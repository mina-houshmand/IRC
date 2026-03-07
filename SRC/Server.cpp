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
//---------------//Getters
int Server::GetPort(){return this->port;}
int Server::GetFd(){return this->server_socket_fd;}

/*The loop iterates through each Client object in the clients vector.
and If a match is found, 
it returns a pointer to the Client object using &this->clients[i].
*/
Client *Server::GetClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].GetFd() == fd)
			return &this->clients[i];
	}
	return NULL;
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


//---------------//Server Methods
//---------------//Parsing Methods
std::vector<std::string> Server::split_recivedBuffer(std::string str)
{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;

	//reads each line from the input stream (stm) into the variable line
	/*
	example:	 "NICK John\r\nUSER johnd 0 * :John Doe\r\nJOIN #channel1\r\n"
	store it like this:
			vec = {"NICK John", "USER johnd 0 * :John Doe", "JOIN #channel1"};
	*/
	while(std::getline(stm, line))
	{
		//find the position of the first occurrence of either '\r' or '\n' in the line
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos); //For each line, it removes the \r\n
		vec.push_back(line);
	}
	return vec;
}

std::vector<std::string> Server::split_cmd(std::string& cmd)
{
	std::vector<std::string> vec;
	std::istringstream stm(cmd);
	std::string token;

	/*     stm >> token
	This code splits the input string (cmd) into individual tokens (words) based on whitespace and stores them in a vector (vec).
	The >> operator reads from the stream until it encounters a whitespace character (space, tab, newline, etc.).
	After extracting the token, the stream's internal pointer moves to the next word.
	The loop continues until the end of the stream is reached.
	{"heloo", "this"}
	*/
	while(stm >> token)
	{
		vec.push_back(token);
		token.clear();
	}
	return vec;
}

// bool Server::notregistered(int fd)
// {
// 	if (!GetClient(fd) || GetClient(fd)->GetNickName().empty() || GetClient(fd)->GetUserName().empty() || GetClient(fd)->GetNickName() == "*"  || !GetClient(fd)->GetLogedIn())
// 		return false;
// 	return true;
// }

bool Server::isregistered(int fd)
{
    Client* client = GetClient(fd);
    if (client && 
        !client->GetNickName().empty() && 
        !client->GetUserName().empty() && 
        client->GetNickName() != "*" && 
        client->GetLogedIn())
    {
        return true;
    }
    return false;
}

void Server::parse_exec_cmd(std::string &cmd, int fd)
{
	if(cmd.empty())
		return ;
	
	//separates the command string into individual words/tokens based on whitespace
	std::vector<std::string> splited_cmd = split_cmd(cmd);

	//returns the index of the first character in cmd that is not one of the characters in the string " \t\v".
	size_t found = cmd.find_first_not_of(" \t\v");

	/*
	The substr method creates a substring of cmd starting from the index found.
	This removes all leading whitespace characters from cmd.
	*/
	if(found != std::string::npos)
		cmd = cmd.substr(found);

	//mina did this
	if (splited_cmd.empty())
		return;

	std::string command = splited_cmd[0];
	for (size_t i = 0; i < command.size(); ++i) {
		command[i] = std::toupper(command[i]);
	}	

	if (splited_cmd.size()){
		if(splited_cmd[0] == "BONG")
			return;
		if(splited_cmd[0] == "PASS" )
			client_authen(fd, cmd);
		else if (splited_cmd[0] == "NICK")
			set_nickname(cmd,fd);
		else if(splited_cmd[0] == "USER" )
			set_username(cmd, fd);
		else if (splited_cmd[0] == "QUIT")
			QUIT(cmd,fd);

		//The IRC protocol requires clients to send PASS, NICK, and USER commands to register with the server.
		//Only after successful registration can clients execute other commands.
		else if(isregistered(fd))
		{
			if (splited_cmd[0] == "KICK")
				KICK(cmd, fd);
			else if (splited_cmd[0] == "JOIN")
				JOIN(cmd, fd);
			else if (splited_cmd[0] == "TOPIC")
				Topic(cmd, fd);
			else if (splited_cmd[0] == "MODE")
				mode_command(cmd, fd);
			else if (splited_cmd[0] == "PART")
				PART(cmd, fd);
			else if (splited_cmd[0] == "PRIVMSG")
				PRIVMSG(cmd, fd);
			else if (splited_cmd[0] == "INVITE")
				Invite(cmd,fd);
			else if (splited_cmd.size())
				_sendResponse(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),splited_cmd[0]),fd);
		}
	}
	else if (!isregistered(fd))
		_sendResponse(ERR_NOTREGISTERED(std::string("*")),fd);
}
