#include "../INC/Server.hpp"

bool Server::isBotfull = false;
Server::Server(){this->server_fdsocket = -1;}
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
		this->server_fdsocket = src.server_fdsocket;
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
int Server::GetFd(){return this->server_fdsocket;}

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
void Server::SetFd(int fd){this->server_fdsocket = fd;}
void Server::SetPort(int port){this->port = port;}
void Server::SetPassword(std::string password){this->password = password;}
std::string Server::GetPassword(){return this->password;}
void Server::AddClient(Client newClient){this->clients.push_back(newClient);}
void Server::AddChannel(Channel newChannel){this->channels.push_back(newChannel);}
void Server::AddFds(pollfd newFd){this->fds.push_back(newFd);}
//---------------//Setters
//---------------//Remove Methods
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
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}
//---------------//Send Methods
//---------------//Close and Signal Methods
bool Server::Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true;
}

void	Server::close_fds(){
	for(size_t i = 0; i < clients.size(); i++){
		std::cout << RED << "Client <" << clients[i].GetFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].GetFd());
	}
	if (server_fdsocket != -1){	
		std::cout << RED << "Server <" << server_fdsocket << "> Disconnected" << WHI << std::endl;
		close(server_fdsocket);
	}
}
//---------------//Close and Signal Methods
//---------------//Server Methods
//Initializes the server and runs the main event loop until a signal tells it to stop
//socket is like a phon when you wanna call s.o
void Server::init(int port, std::string pass)
{
	this->password = pass;
	this->port = port;
	this->set_sever_socket();

	std::cout << GRE << "Server <" << server_fdsocket << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	while (Server::Signal == false)
	{
		/*
		several clienets at the same time -> poll() solves this by:
		Watching many sockets at once
		Sleeping until something happens
		Waking up exactly when needed

		poll() is like telling your operating system:
		“Hey, OS, watch all these clients for me.
		Wake me up only when someone needs attention.”
		New person at the desk → wakes you
		Someone calling → wakes you
		Ctrl+C signal → wakes you to shut down
		💡 The server sleeps until something happens,
		 then poll() tells it exactly which client or socket needs attention.
		*/

		/*
		what poll() do --> Don’t call every client every second.
		Sit down. I’ll tap you on the shoulder when someone needs you.”
		*/

		/*
		poll return value-->
		poll() only tells which sockets are ready
		> 0 -->Number of sockets that have events (ready to read/write)
		0   --> Timeout occurred (not used here because -1)
		-1  --> Error occurred (or signal interrupted it)
		*/

		/*
		struct pollfd {
			int   fd;       // the socket/file descriptor
			short events;   // what we want to watch (input/output)
			short revents;  // what actually happened (set by poll)
			
		};

		When poll returns, it fills each socket’s revents field with what happened
		short -> a small integer type (16 bits)
		POLLIN → ready to read (This socket has data available to read)

		POLLOUT → ready to write

		POLLERR → error

		POLLHUP -> Socket is hung up / disconnected
		*/

		/*
		int poll(struct pollfd *fds, nfds_t nfds, int timeout);

		struct pollfd *fds  → the list/array of sockets (file descriptors) you want to watch
		nfds_t nfds			→ the number of sockets in that list
		int timeout 		→ how long to wait before giving up
		-1 --> wait forever → block until something happens
		*/
		
		/*
		ok here in this line the code is blocked and poll will wait for an event then if sth happends the poll will continue the code and we will check which fd had the event
		*/
		if((poll(&fds[0], fds.size(), -1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));

		
		for (size_t i = 0; i < fds.size(); i++)
		{
			//here we wanna check if there is an event on this fd
			//by checking if bitwise of POLLIN and revents is 1,
			//If POLLIN is set in revents, it means the socket is ready to read.
			//so POLLIN is 0x0001 and bitwise with revents if revents is also 0x0001 the result will be 0x0001 which is true

			/*
			POLLIN is set in revents when the file descriptor is ready to read. This means:
			For the server socket (server_fdsocket): A new client is trying to connect.
			For a client socket: The client has sent data that is ready to be read.
			*/

			/*
			Your fds vector contains pollfd structures for all sockets the server cares about.
			At runtime, fds looks like this:
					fds[0] → listening socket (server_fdsocket)
					fds[1] → client socket #1
					fds[2] → client socket #2
					fds[3] → client socket #3
					...
			What poll() does
				poll() waits until something happens on any socket in fds
				When it returns:
					It sets revents for the sockets that had activity
			You then loop through fds to see which sockets need attention.
			and check any socket is ready for reading or has an event.” -> (fds[i].revents & POLLIN)

			POLLIN actually means -> This fd is readable without blocking
			So the meaning of POLLIN is determined by the role of the socket, not by poll() itself.

			so each time the event happens in a server socket that means a new client is trying to connect
			so we check the fd to find out if it is a server socket so we need to creat new client
			but if the fd was for client socket so we need to read data from this client

			in totall :
			we have a one listening socket in the server side
			which it is responsible to listen and if saw a new client trying to connect 
			it will create a client socket (with accept()) and then it will listen again for another client
			The listening socket does not talk to clients.
			It only creates client sockets.


			*/
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == server_fdsocket)
					this->accept_new_client();
				else
					this->reciveNewData(fds[i].fd);
			}
		}
	}
	close_fds();
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
	server_fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fdsocket == -1)
		throw(std::runtime_error("faild to create socket"));
	
	//set rules to this socket
	/*
	-server_fdsocket ->which socket we are configuring
	-SOL_SOCKET ->Set options at the socket level
	-SO_REUSEADDR ->Allows the server to reuse the port immediately after closing, instead of waiting for the OS to free it.
	*/
	if(setsockopt(server_fdsocket, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
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
	if (fcntl(server_fdsocket, F_SETFL, O_NONBLOCK) == -1)
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
	if (bind(server_fdsocket, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));

	//Put the socket into listening mode to accept incoming connections.
	//SOMAXCONN -> maximum number of pending connections allowed

	if (listen(server_fdsocket, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));

	/*
	Step-by-step flow

	Create listening socket → server_fdsocket
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
	new_cli.fd = server_fdsocket;
	new_cli.events = POLLIN;
	new_cli.revents = 0;

	//fds is a list of all sockets we want to monitor
	fds.push_back(new_cli);
}


void Server::accept_new_client()
{
	Client cli;

	//Clear client address structure and Avoids garbage data
	memset(&cliadd, 0, sizeof(cliadd));
	socklen_t len = sizeof(cliadd);

	/* in accept():
	- OS checks the listening socket
	- Takes one pending client
	- Creates a new socket
	Returns:
		- a new FD (incofd)
		- fills cliadd with client IP & port
	*/
	int incofd = accept(server_fdsocket, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}

	//Set the new client socket to non-blocking mode.
	//This is mandatory when using poll().
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
		{std::cout << "fcntl() failed" << std::endl; return;}


	/*Tell poll():
		- watch this client socket
		-  notify when it sends data or disconnects
	*/
	new_cli.fd = incofd;
	new_cli.events = POLLIN;
	new_cli.revents = 0;
	cli.SetFd(incofd);

	/*
		cliadd.sin_addr → client IP (binary form)
		inet_ntoa() 	→ converts it to readable string
		setIpAdd() 		→ stores it in the client object
	*/
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));

	//Adds the client to your server’s client list
	//Server now knows this client exists
	clients.push_back(cli);

	//Adds the client socket to the pollfd list to monitor it
	/*Now poll() will:
		monitor this client
		notify when it sends data or disconnects
	*/
	fds.push_back(new_cli);
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::reciveNewData(int fd)
{
	std::vector<std::string> cmd;
	/*
	what if the data is bigger than the buffer size??
	it gonna go back to the poll and
	The poll() function will detect that the socket is still readable.
	The POLLIN event will be set for that socket in the next poll() call.
	The server will call reciveNewData() for that socket again to read the remaining data
	*/
	char buff[1024];

	//Fills the buffer with 0 bytes. to clear it
	memset(buff, 0, sizeof(buff));
	Client *cli = GetClient(fd);

	//recv() reads data from the client socket and stores it in buff
	/*
	ir returns:
		>0 number of bytes successfully received from the socket.
		0 client disconnected
		<0 error
	*/
	//The -1 ensures there is space for a null terminator (\0) if the data is treated as a string.
	// 0 -> means no special flags are set, and the function operates in its default mode.
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{
		if (bytes == 0)
			std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		else 
			std::cerr << RED << "Error receiving data from Client <" << fd << ">: " << strerror(errno) << WHI << std::endl;
		RmChannels(fd);
		RemoveClient(fd);
		RemoveFds(fd);
		close(fd);
	}
	else
	{ 
		cli->setBuffer(buff);
		if(cli->getBuffer().find_first_of("\r\n") == std::string::npos)
			return;
		cmd = split_recivedBuffer(cli->getBuffer());
		for(size_t i = 0; i < cmd.size(); i++)
			this->parse_exec_cmd(cmd[i], fd);
		if(GetClient(fd))
			GetClient(fd)->clearBuffer();
	}
}
//---------------//Server Methods
//---------------//Parsing Methods
std::vector<std::string> Server::split_recivedBuffer(std::string str)
{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;
	while(std::getline(stm, line))
	{
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos);
		vec.push_back(line);
	}
	return vec;
}

std::vector<std::string> Server::split_cmd(std::string& cmd)
{
	std::vector<std::string> vec;
	std::istringstream stm(cmd);
	std::string token;
	while(stm >> token)
	{
		vec.push_back(token);
		token.clear();
	}
	return vec;
}

bool Server::notregistered(int fd)
{
	if (!GetClient(fd) || GetClient(fd)->GetNickName().empty() || GetClient(fd)->GetUserName().empty() || GetClient(fd)->GetNickName() == "*"  || !GetClient(fd)->GetLogedIn())
		return false;
	return true;
}

void Server::parse_exec_cmd(std::string &cmd, int fd)
{
	if(cmd.empty())
		return ;
	std::vector<std::string> splited_cmd = split_cmd(cmd);
	size_t found = cmd.find_first_not_of(" \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	if(splited_cmd.size() && (splited_cmd[0] == "BONG" || splited_cmd[0] == "bong"))
		return;
    if(splited_cmd.size() && (splited_cmd[0] == "PASS" || splited_cmd[0] == "pass"))
        client_authen(fd, cmd);
	else if (splited_cmd.size() && (splited_cmd[0] == "NICK" || splited_cmd[0] == "nick"))
		set_nickname(cmd,fd);
	else if(splited_cmd.size() && (splited_cmd[0] == "USER" || splited_cmd[0] == "user"))
		set_username(cmd, fd);
	else if (splited_cmd.size() && (splited_cmd[0] == "QUIT" || splited_cmd[0] == "quit"))
		QUIT(cmd,fd);
	else if(notregistered(fd))
	{
		if (splited_cmd.size() && (splited_cmd[0] == "KICK" || splited_cmd[0] == "kick"))
			KICK(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "JOIN" || splited_cmd[0] == "join"))
			JOIN(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "TOPIC" || splited_cmd[0] == "topic"))
			Topic(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "MODE" || splited_cmd[0] == "mode"))
			mode_command(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PART" || splited_cmd[0] == "part"))
			PART(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PRIVMSG" || splited_cmd[0] == "privmsg"))
			PRIVMSG(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "INVITE" || splited_cmd[0] == "invite"))
			Invite(cmd,fd);
		else if (splited_cmd.size())
			_sendResponse(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),splited_cmd[0]),fd);
	}
	else if (!notregistered(fd))
		_sendResponse(ERR_NOTREGISTERED(std::string("*")),fd);
}
//---------------//Parsing Methods