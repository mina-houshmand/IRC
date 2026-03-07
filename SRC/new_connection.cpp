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
        printError("Failed : to accept() the new  connection: ");
        return -1;
    }
    
    return clientFd;
}
//Set the new client socket to non-blocking mode.
//This is mandatory when using poll().
bool Server::setupClientSocket(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1){
        printError("Failed : fcntl() to set new client socket to non-blocking mode");
        close(fd);
        return false;  // ← Just that client failed
    }
    return true;
}

/*Tell poll():
    - watch this client socket
    -  notify when it sends data or disconnects
*/
//Adds the client socket to the pollfd list to monitor it
/*Now poll() will:
    monitor this client
    notify when it sends data or disconnects
*/
void Server::addClientToPoll(int fd)
{
    new_cli.fd = fd;
    new_cli.events = POLLIN;
    new_cli.revents = 0;
    fds.push_back(new_cli);
}

/*
cliadd.sin_addr → client IP (binary form)
inet_ntoa() 	→ converts it to readable string
setIpAdd() 		→ stores it in the client object
*/
void Server::initializeClient(Client &client, int fd){
    client.SetClient_Fd(fd);
    client.set_IpAddress(cliadd.sin_addr);
}

//Adds the client to your server’s client list
//Server now knows this client exists
void Server::addClientToServer(const Client &client){
    clients.push_back(client);
}


void Server::new_connection_request()
{
	Client client;

    int client_fd = acceptNewClient();
    if (client_fd == -1)
        return;
    if (!setupClientSocket(client_fd))
        return;
    addClientToPoll(client_fd);
    initializeClient(client, client_fd);
    addClientToServer(client);
	printStatus(GRE, "Client", client_fd, "Connected");
}
