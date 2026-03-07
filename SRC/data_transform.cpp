#include "../INC/Server.hpp"

void Server::processClientCommands(const std::string &buffer, int fd)
{
    std::vector<std::string> commands = split_recivedcmd(buffer);
    
    for (size_t i = 0; i < commands.size(); i++) {
        if (!commands[i].empty())
            this->handleClientCommand(commands[i], fd);
    }
}

/*
what if the data is bigger than the buffer size??
it gonna go back to the poll and
The poll() function will detect that the socket is still readable.
The POLLIN event will be set for that socket in the next poll() call.
The server will call data_transform() for that socket again to read the remaining d/ata

recv() reads data from the client socket and stores it in buff

ir returns:
	>0 number of bytes successfully received from the socket.
	0 client disconnected
	<0 error

The -1 ensures there is space for a null terminator (\0) if the data is treated as a string.
0 -> means no special flags are set, and the function operates in its default mode.

*/

void Server::data_transform(int fd)
{
	std::vector<std::string> cmd;

	char buff[1024];
	memset(buff, 0, sizeof(buff));
	                               //**change name to GetClientByFd
	Client *cli = GetClient(fd);
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{
		if (bytes == 0)
			printStatus(RED, "Client", fd, "Disconnected");
		else 
			std::cerr << RED << "Error receiving data from Client <" << fd << ">: " << strerror(errno) << WHI << std::endl;
		disconnectClient(fd);
	}
	else
	{ 
		cli->addToBuffer(buff);
		// Wait for complete command (must end with \r\n)
		if(cli->GetCmds().find_first_of("\r\n") == std::string::npos)
			return;
		
		processClientCommands(cli->GetCmds(), fd);


		//After processing all complete commands, the server clears the client's buffer to prepare for new incoming data.
		if(GetClient(fd))
			GetClient(fd)->clearBuffer();
	}
}
