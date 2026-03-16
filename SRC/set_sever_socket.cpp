#include "../INC/Server.hpp"

/*
what is a socket?

A socket is like a phone for programs.
Your server is a program that waits for calls
A client is a program that calls it
The network (IP + port) is the phone number
The socket is the phone device itself



1-Create a socket → get a phone
2-Configure the socket → set rules for the phone
3-Bind → give it an address (IP + port)
4-Listen → start waiting for calls
5-Accept → pick up calls (not shown yet in your code)
Your function does steps 1–4.



server_socket = socket(AF_INET, SOCK_STREAM, 0);
server_socket holds a valid FD for the socket you created

-What kind of address will this socket use? --> AF_INET → IPv4 (most common)
												AF_INET6 → IPv6
-How should data be sent?	-->		SOCK_STREAM → TCP (reliable, ordered)
									SOCK_DGRAM → UDP (fast, unreliable)
-protocol --> usually 0 to choose the default for the type



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


	Step-by-step flow

	Create listening socket → server_socket_fd
	Add it to pollfd (new_cli) to watch for incoming connections
	Call poll() → OS tells you a client is trying to connect
	Call accept() → returns new client socket FD
	Add that new client FD to pollfd list → now you can monitor it for data

	//here we created the socket and we are adding it to the pollfd list
	//and then we add each socket to the fds vector to monitor it
	//then in the main loop we call poll on the fds vector

	Tell poll():
		- watch this socket
		-  notify when it sends data or disconnects
	
*/

void Server::addSocketToPoll(int fd)
{
    new_cli.fd = fd;
    new_cli.events = POLLIN;
    new_cli.revents = 0;
    fds.push_back(new_cli);
}

void Server::initializeAddress(struct sockaddr_in &address, int port) {
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;     //Accept connections on any network interface
    address.sin_port = htons(port);
}

void Server::set_sever_socket()
{
	int enable = 1;

	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket_fd == -1)
		throw(std::runtime_error("Failed to create socket"));
	
	if(setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
		throw(std::runtime_error("Failed to set socket option SO_REUSEADDR"));
	
	if (fcntl(server_socket_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("Failed to set socket to non-blocking mode"));

    initializeAddress(add, port);

	if (bind(server_socket_fd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("Failed to bind socket"));

	if (listen(server_socket_fd, SOMAXCONN) == -1)
		throw(std::runtime_error("Failed to listen on socket"));

    addSocketToPoll(server_socket_fd);

}

/*
----------------------------------------------------------------
setsockopt()

-SOL_SOCKET ->Set options at the socket level
-SO_REUSEADDR ->Allows the server to reuse the port immediately after closing, instead of waiting for the OS to free it.

----------------------------------------------------------------
fcntl()
fcntl stands for file control.
Sockets are treated like files in Unix/Linux
change the behavior of the socket.

F_SETFL --> Set file status flags
O_NONBLOCK --> Non-blocking mode

By default, sockets are blocking:
    - accept() waits forever for a client.
    - recv() waits forever for data.
With non-blocking:
    -accept() immediately returns if no client is connecting.
    -recv() immediately returns if no data is available.
Perfect for servers handling multiple clients at once.

------------------------------------------------------------------
bind()

Assign a specific IP address and port to your socket.
bind() connects the socket to the address you prepared earlier.
“Give this socket a phone number so clients can call it.”
we cast the structure with -> (struct sockaddr *) 
bind() expects a generic address type (struct sockaddr *)

Common reasons to bind can fail:
    Port already in use
    trying to bind port <1024 without root

------------------------------------------------------------------
listen()

Put the socket into listening mode to accept incoming connections.
SOMAXCONN -> maximum number of pending connections allowed


*/