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


-What kind of address will this socket use? --> AF_INET → IPv4 (most common)
						AF_INET6 → IPv6
-How should data be sent?	            --> SOCK_STREAM → TCP (reliable, ordered)
						SOCK_DGRAM → UDP (fast, unreliable)
-protocol --> usually 0 to choose the default for the type



struct sockaddr_in {
    sa_family_t    sin_family;   // Address family IPv4 or IPv6
    in_port_t      sin_port;     // Port number
    struct in_addr sin_addr;     // IP address
};


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
    address.sin_port = htons(port);	// Convert port to network format
}

/*
to accept incoming connections:

Create Socket → socket() returns file descriptor
Configure → setsockopt() (reuse port), fcntl() (non-blocking)
Bind → bind() Attach the IP address - port to the socket
Listen → listen() enters listening mode
Poll → addSocketToPoll() adds to monitoring list
*/


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
