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



/*
what is a socket?

A socket is like a phone for programs.
Your server is a program that waits for calls
A client is a program that calls it
The network (IP + port) is the phone number
The socket is the phone device itself
*/

/*
1-Create a socket → get a phone
2-Configure the socket → set rules for the phone
3-Bind → give it an address (IP + port)
4-Listen → start waiting for calls
5-Accept → pick up calls (not shown yet in your code)
Your function does steps 1–4.
*/

/*
server_socket = socket(AF_INET, SOCK_STREAM, 0);
server_socket holds a valid FD for the socket you created

-What kind of address will this socket use? --> AF_INET → IPv4 (most common)
												AF_INET6 → IPv6
-How should data be sent?	-->		SOCK_STREAM → TCP (reliable, ordered)
									SOCK_DGRAM → UDP (fast, unreliable)
-protocol --> usually 0 to choose the default for the type
*/

/*
where should this socket live?
That’s where structures come in.

#include <netinet/in.h>

struct sockaddr_in server_address;

This structure stores:
-IP version
-IP address
-Port number

struct sockaddr_in {
    sa_family_t    sin_family;   // Address family IPv4 or IPv6
    in_port_t      sin_port;     // Port number
    struct in_addr sin_addr;     // IP address
};



*/

/*

After creating the socket and preparing its address structure with 
the desired IP and port, the socket must be configured using socket 
options such as address reuse and non-blocking mode. Once configured,
the socket can be bound to the address and placed into listening mode

to accept incoming connections.
1-Create socket → get FD
2-Prepare address → IP + port
3-Configure socket → options + behavior
4-Bind → attach address
5-Listen → accept connections



🧱 Built the socket (socket()) 
📄 Got its file descriptor (FD)
🏷 Prepared its address (sockaddr_in)
⚙️ Configured the socket (setsockopt(), fcntl())
📎 Bound the socket to IP and port (bind())
🚪 Put the socket into listening mode (listen())
⏳ Waited for incoming connections (poll() / select())
🤝 Accepted a client connection (accept())
📡 Exchanged data with the client (send(), recv())
❌ Closed the client socket (close())
🔒 Closed the server socket (close())

*/
void Server::set_sever_socket()
{
	//this is like a switch button
	int en = 1;

	/*
	add.sin_family = AF_INET :
	Must match the socket ->
	-Socket → AF_INET
	-Address → AF_INET
	*/
	add.sin_family = AF_INET;

	//Accept connections on any network interface
	add.sin_addr.s_addr = INADDR_ANY;

	// from #include <arpa/inet.h>
	//converts port to → network byte order
	//network order is big-endian like convert 8080 to hex : 0x1F90 and convert to big-endian : 1F 90
	add.sin_port = htons(port);

	//create a socket and it returns fd
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket_fd == -1)
		throw(std::runtime_error("faild to create socket"));
	
	//set rules to this socket
	/*
	-server_socket_fd ->which socket we are configuring
	-SOL_SOCKET ->Set options at the socket level
	-SO_REUSEADDR ->Allows the server to reuse the port immediately after closing, instead of waiting for the OS to free it.
	*/
	if(setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	
	//fcntl stands for file control.
	//Sockets are treated like files in Unix/Linux
	//change the behavior of the socket.
	/*
	F_SETFL --> Set file status flags
	O_NONBLOCK --> Non-blocking mode

	By default, sockets are blocking:
		- accept() waits forever for a client.
		- recv() waits forever for data.
	With non-blocking:
		-accept() immediately returns if no client is connecting.
		-recv() immediately returns if no data is available.
	Perfect for servers handling multiple clients at once.
	*/
	if (fcntl(server_socket_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));

	//Assign a specific IP address and port to your socket.
	//bind() connects the socket to the address you prepared earlier.
	// “Give this socket a phone number so clients can call it.”
	// we cast the structure with -> (struct sockaddr *) 
	//bind() expects a generic address type (struct sockaddr *)
	/*
	Common reasons to bind can fail:
		Port already in use
		trying to bind port <1024 without root
	*/
	if (bind(server_socket_fd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));

	//Put the socket into listening mode to accept incoming connections.
	//SOMAXCONN -> maximum number of pending connections allowed

	if (listen(server_socket_fd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));

	/*
	Step-by-step flow

	Create listening socket → server_socket_fd
	Add it to pollfd (new_cli) to watch for incoming connections
	Call poll() → OS tells you a client is trying to connect
	Call accept() → returns new client socket FD
	Add that new client FD to pollfd list → now you can monitor it for data
	*/

	//here we created the socket and we are adding it to the pollfd list
	//and then we add each socket to the fds vector to monitor it
	//then in the main loop we call poll on the fds vector

	/*Tell poll():
		- watch this socket
		-  notify when it sends data or disconnects
	*/
	new_cli.fd = server_socket_fd;
	new_cli.events = POLLIN;
	new_cli.revents = 0;

	//fds is a list of all sockets we want to monitor
	fds.push_back(new_cli);
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
