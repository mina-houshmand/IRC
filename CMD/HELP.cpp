#include "../INC/Server.hpp"
#include <iostream>

/*
HELP COMMAND
The HELP command provides users with information about available commands and their usage. 
When a client sends the HELP command, the server responds with a list of supported commands 
along with brief descriptions of their functionality. 
This allows users to understand how to interact with the server and utilize its features effectively.
*/

void Server::HELP(int fd)
{
	std::string helpMessage = "Available commands:\r\n"
							"PASS <password> - Authenticate with the server\r\n"
							"NICK <nickname> - Set your nickname\r\n"
							"USER <username> <hostname> <servername> :<realname> - Set your username and real name\r\n"
							  "JOIN <channel>{,<channel>} [<key>{,<key>}] - Join a channel\r\n"
							  "PART <channel>{,<channel>} [<message>] - Leave a channel\r\n"
							  "PRIVMSG <target> :<message> - Send a private message\r\n"
							  "NICK <nickname> - Change your nickname\r\n"
							  "USER <username> <hostname> <servername> :<realname> - Set your username and real name\r\n"
							  "QUIT [<message>] - Disconnect from the server\r\n"
							  "HELP - Show this help message\r\n";
	_sendResponse(helpMessage, fd);
}