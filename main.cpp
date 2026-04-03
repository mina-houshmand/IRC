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
    if (password.empty())
    {return false;}
	
	bool isonlywhiltespace = true;
	for(size_t i = 0; i < password.length(); ++i) {
		if (!std::isspace(password[i])) {
			isonlywhiltespace = false;
			break;
		}
	}
	if (isonlywhiltespace)
		return false;
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
	bool bonus = false;
	if (ac != 3 && ac != 4)
	{
		ser.printError("Error: Invalid number of arguments!");
		ser.printMessage("Usage: " + std::string(av[0]) + " <port number> <password> [--bonus]");
		return 1;
	}

	if (ac == 4) {
		std::string flag = av[3];
		if (flag == "--bonus")
			bonus = true;
		else {
			ser.printMessage("Unknown flag: " + flag);
			return 1;
		}
	}

	ser.printMessage("---- SERVER ----");
	try
	{
		setupSignals();
		
		if (!isPortValid(av[1]))
		{
			ser.printError("Error: Invalid port number!");
			ser.printMessage("Port must be between 1024-65535 (digits only)");
			return 1;
		}
		
		if (!isPasswordValid(av[2]))
		{
			ser.printError("Error: Invalid password!");
			ser.printMessage("Password must be 1-20 characters & cannot be only whitespace");
			return 1;
		}

		if (bonus)
			ser.EnableTriviaBot();

		ser.server_config(av[1], av[2]);
	}
	catch(const std::exception& e)
	{
		ser.close_fds();
		std::cerr << e.what() << std::endl;
	}
	ser.printMessage("The Server Closed!");
}
