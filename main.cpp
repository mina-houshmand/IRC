#include "INC/Server.hpp"

// ============= PORT VALIDATION =============
// Rule 1: Port must contain ONLY digits (0-9)
// Rule 2: Port range: 1024 - 65535
// Why 1024+? Ports below 1024 are reserved for system services

//find_first_not_of -> Returns the index of the first non-digit character
//if it returns npos that means there was n't any thing to find
bool isPortValid(std::string port)
{
	const int MIN_PORT = 1024;
	const int MAX_PORT = 65535;
	
	// Check 1: Must contain only digits
	bool isOnlyDigits = (port.find_first_not_of("0123456789") == std::string::npos);
	if (!isOnlyDigits)
		return false;
	
	// Check 2: Must be within valid port range
	int portNum = std::atoi(port.c_str());
	bool isInRange = (portNum >= MIN_PORT && portNum <= MAX_PORT);
	
	return isInRange;
}

// ============= PASSWORD VALIDATION =============
// Rule 1: Password must NOT be empty
// Rule 2: Password length must be <= 20 characters
// Why? Prevents buffer overflow and abuse

bool isPasswordValid(std::string password)
{
	const int MAX_PASSWORD_LENGTH = 20;
	
	// Check 1: Password is not empty
	if (password.empty())
		return false;
	
	// Check 2: Password length <= 20
	bool isValidLength = (password.length() <= MAX_PASSWORD_LENGTH);
	
	return isValidLength;
}

void setupSignals()
{
	signal(SIGINT, Server::SignalHandler);  //SIGINT → Ctrl + C
	signal(SIGQUIT, Server::SignalHandler);  //SIGQUIT → Ctrl +/
	signal(SIGPIPE, SIG_IGN); // or MSG_NOSIGNAL flag in send() to ignore SIGPIPE on linux systems
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
		setupSignals();
		
		if (!isPortValid(av[1]))
		{
			std::cout << "Error: Invalid port number!" << std::endl;
			std::cout << "Port must be between 1024-65535 (digits only)" << std::endl;
			return 1;
		}
		
		if (!isPasswordValid(av[2]))
		{
			std::cout << "Error: Invalid password!" << std::endl;
			std::cout << "Password must be 1-20 characters" << std::endl;
			return 1;
		}
		
		ser.init(av[1], av[2]);
	}
	catch(const std::exception& e)
	{
		ser.close_fds();
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}
