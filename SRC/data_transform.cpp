#include "../INC/Server.hpp"

void Server::data_transform(int fd)
{
	std::vector<std::string> cmd;
	/*
	what if the data is bigger than the buffer size??
	it gonna go back to the poll and
	The poll() function will detect that the socket is still readable.
	The POLLIN event will be set for that socket in the next poll() call.
	The server will call data_transform() for that socket again to read the remaining data
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
			printStatus(RED, "Client", fd, "Disconnected");
		else 
			std::cerr << RED << "Error receiving data from Client <" << fd << ">: " << strerror(errno) << WHI << std::endl;
		RmChannels(fd);
		RemoveClient(fd);
		RemoveFds(fd);
		close(fd);
	}
	else
	{ 
		//the std::string constructor is automatically called to convert the char[] into a std::string.
		//std::string(const char* s); SO we can pass char[] to setBuffer
		cli->setBuffer(buff);

		//\r\n  to detect the end of a command sent by the client.
		//The IRC protocol specifies that commands must end with \r\n.
		//If the buffer does not contain \r\n, the server assumes the command is incomplete and waits for more data.
		if(cli->getBuffer().find_first_of("\r\n") == std::string::npos)
			return;

		
		cmd = split_recivedBuffer(cli->getBuffer());
		for(size_t i = 0; i < cmd.size(); i++)
			this->parse_exec_cmd(cmd[i], fd);

		//After processing all complete commands, the server clears the client's buffer to prepare for new incoming data.
		if(GetClient(fd))
			GetClient(fd)->clearBuffer();
	}
}
