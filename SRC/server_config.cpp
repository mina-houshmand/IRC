#include "../INC/Server.hpp"

//if we dont check the signal -> This would throw an 
// error even when the user pressed Ctrl+C (which is normal).

int Server::performPoll_request() {
    int pollResult = poll(&fds[0], fds.size(), -1);
    if (pollResult == -1 && !Server::Signal) {
        throw std::runtime_error("Failed: poll request");
    }
    return pollResult;
}

bool Server::isSocketReadable(const pollfd& pfd) {
    int readFlag = pfd.revents & POLLIN;  // Extract POLLIN bit
    bool hasData = (readFlag != 0);        // Check if it's set
    return hasData;
}

/*
several clienets at the same time ->
What poll() does
    poll() waits until something happens on any socket in fds
    When it returns:
        It sets revents for the sockets that had activity


poll return value-->
poll() only tells which sockets are ready
    > 0 -->Number of sockets that have events (ready to read/write)
    0   --> Timeout occurred (not used here because -1)
    -1  --> Error occurred (or signal interrupted it)


struct pollfd {
    int   fd;       // the socket/file descriptor
    short events;   // what we want to watch (input/output)
    short revents;  // what actually happened (set by poll)
    
};

POLLIN → ready to read (This socket has data available to read)

POLLOUT → ready to write

POLLERR → error

POLLHUP -> Socket is hung up / disconnected


 poll() implementationL:
int poll(struct pollfd *fds, nfds_t nfds, int timeout);

struct pollfd *fds  → the list/array of sockets (file descriptors) you want to watch
nfds_t nfds			→ the number of sockets in that list
int timeout 		→ how long to wait before giving up
-1 --> wait forever → block until something happens

in poll() line the code is gonna blocked and waite for connection.



here we wanna check if there is an event on this fd
by checking if bitwise of POLLIN and revents is 1,
If POLLIN is set in revents, it means the socket is ready to read.
so POLLIN is 0x0001 and bitwise with revents if revents is also
not 0 the result will be 0x0001 which is true


Your fds vector contains pollfd structures for all sockets the server cares about.
At runtime, fds looks like this:
        fds[0] → listening socket (server_socket_fd)
        fds[1] → client socket #1
        fds[2] → client socket #2
        fds[3] → client socket #3
        ...


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

void Server::server_config(std::string port, std::string pass)
{
	this->password = pass;
	this->port = std::atoi(port.c_str());
	this->set_sever_socket();

	printStatus(GRE, "Server", server_socket_fd, "Connected");
	printMessage("Waiting to accept a connection...");	
	while (Server::Signal == false)
	{
		int pollResult = performPoll_request();
		if (pollResult > 0) {		
			for (size_t i = 0; i < fds.size(); i++)
			{
				if (isSocketReadable(fds[i]))
				{
					if (fds[i].fd == server_socket_fd)
						this->new_connection_request();
					else
						this->data_transform(fds[i].fd);
				}
			}
		}
	}
	close_fds();
}

