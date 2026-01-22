#include "INC/Server.hpp"

/*
isPortValid(av[1])

Checks:
Only digits
Port range: 1024–65535
*/

//!*av[2] --> Password is not empty ("")

/*
std::strlen(av[2]) > 20
Limits password length
Prevents abuse / buffer issues
*/

bool isPortValid(std::string port){
	return (port.find_first_not_of("0123456789") == std::string::npos && \
	std::atoi(port.c_str()) >= 1024 && std::atoi(port.c_str()) <= 65535);
}

int main(int ac, char **av)
{
	Server ser;
	if (ac != 3)
		{
			std::cout << "Usage: " << av[0] << " <port number> <password>" << std::endl; 
			return 1;
		}
	std::cout << "---- SERVER ----" << std::endl;
	try
	{
		signal(SIGINT, Server::SignalHandler);  //SIGINT → Ctrl + C
		signal(SIGQUIT, Server::SignalHandler);  //SIGQUIT → Ctrl +/
		signal(SIGPIPE, SIG_IGN); // or MSG_NOSIGNAL flag in send() to ignore SIGPIPE on linux systems
		if(!isPortValid(av[1]) || !*av[2] || std::strlen(av[2]) > 20)
			{std::cout << "invalid Port number / Password!" << std::endl; return 1;}
		ser.init(std::atoi(av[1]), av[2]);
	}
	catch(const std::exception& e)
	{
		ser.close_fds();
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}
