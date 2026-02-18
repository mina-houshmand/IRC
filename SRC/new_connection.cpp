#include "../INC/Server.hpp"

/* in accept():
- OS checks the listening socket
- Takes one pending client
- Creates a new socket
Returns:
    - a new FD (incofd)
    - fills cliadd with client IP & port
*/
int Server::acceptNewClient()
{
    memset(&cliadd, 0, sizeof(cliadd));
    socklen_t addrLen = sizeof(cliadd);
    
    int clientFd = accept(server_socket_fd, (sockaddr *)&cliadd, &addrLen);
    
    if (clientFd == -1) {
        printError("accept() failed: " + std::string(strerror(errno)));
        return -1;
    }
    
    return clientFd;
}
//Set the new client socket to non-blocking mode.
//This is mandatory when using poll().
void Server::setupClientSocket(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1){
        printMessage("fcntl() failed");
        close(fd);
        printError("Failed to set non-blocking mode");
    }
}

/*Tell poll():
    - watch this client socket
    -  notify when it sends data or disconnects
*/
void Server::addClientToPoll(int fd)
{
    new_cli.fd = fd;
    new_cli.events = POLLIN;
    new_cli.revents = 0;
}

void Server::new_connection_request()
{
	Client client;

    int incofd = acceptNewClient();
    if (incofd == -1)
        return;
    setupClientSocket(incofd);
    addClientToPoll(incofd);
    client.SetFd(incofd);
	/*
    cliadd.sin_addr → client IP (binary form)
    inet_ntoa() 	→ converts it to readable string
    setIpAdd() 		→ stores it in the client object
	*/
	client.setIpAdd(inet_ntoa((cliadd.sin_addr)));

	//Adds the client to your server’s client list
	//Server now knows this client exists
	clients.push_back(client);

	//Adds the client socket to the pollfd list to monitor it
	/*Now poll() will:
		monitor this client
		notify when it sends data or disconnects
	*/
	fds.push_back(new_cli);
	printStatus(GRE, "Client", incofd, "Connected");
}
