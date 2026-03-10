#include "../INC/Server.hpp"

void Server::cmd_toUpper(std::string &command){
    for (size_t i = 0; i < command.size(); ++i) {
        command[i] = std::toupper(command[i]);
    }	
}
void Server::trimLeadingWhitespace(std::string &cmd)
{
    size_t firstChar = cmd.find_first_not_of(" \t\v");
    if (firstChar != std::string::npos)
        cmd = cmd.substr(firstChar);
}

/*
1 -split_cmd : separates the command string into individual words/tokens based on whitespace
2- found : finds the index of the first non-whitespace character in the command string
3 - trimLeadingWhitespace : removes any leading whitespace from the command string, ensuring that the command is properly formatted for processing.
*/
void Server::handleClientCommand(std::string &cmd, int fd)
{
	std::string trimmedCmd = cmd;
    trimLeadingWhitespace(trimmedCmd);

	std::vector<std::string> tokens = split_cmd(cmd);
	if (tokens.empty())
		return;

	std::string command = tokens[0];
	cmd_toUpper(command);

	// Check if client exists before processing the command
    Client *client = GetClient(fd);
    if (client == NULL){
        return;
	}
	
	if(command == "BONG")
		return;
	if(command == "PASS" )
		client_authen(fd, cmd);
	else if (command == "NICK")
		set_nickname(cmd,fd);
	else if(command == "USER" )
		set_username(cmd, fd);
	else if (command == "QUIT")
		QUIT(cmd,fd);

	//The IRC protocol requires clients to send PASS, NICK, and USER commands to register with the server.
	//Only after successful registration can clients execute other commands.
	else if(isClientRegistered(fd))
	{
		if (command == "KICK")
			KICK(cmd, fd);
		else if (command == "JOIN")
			JOIN(cmd, fd);
		else if (command == "TOPIC")
			Topic(cmd, fd);
		else if (command == "MODE")
			mode_command(cmd, fd);
		else if (command == "PART")
			PART(cmd, fd);
		else if (command == "PRIVMSG")
			PRIVMSG(cmd, fd);
		else if (command == "INVITE")
			Invite(cmd,fd);
		else
			_sendResponse(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),command),fd);
	}
	else if (!isClientRegistered(fd))
		_sendResponse(ERR_NOTREGISTERED(std::string("*")),fd);
}

/*reads each line from the input stream (stm) into the variable line

example:	 "NICK John\r\nUSER johnd 0 * :John Doe\r\nJOIN #channel1\r\n"
store it like this:
		vec = {"NICK John", "USER johnd 0 * :John Doe", "JOIN #channel1"};
*/

std::vector<std::string> Server::split_recivedcmd(std::string str)
{
    std::vector<std::string> vec;
    
    if (str.empty())
        return vec;
    
    std::istringstream stm(str);
    std::string line;

    while(std::getline(stm, line))
    {
        size_t pos = line.find_first_of("\r\n");
        if(pos != std::string::npos)
            line = line.substr(0, pos);
        
        if (!line.empty())
            vec.push_back(line);
    }
    return vec;
}

/*     stm >> token
This code splits the input string (cmd) into individual tokens (words) based on whitespace and stores them in a vector (vec).
The >> operator reads from the stream until it encounters a whitespace character (space, tab, newline, etc.).
After extracting the token, the stream's internal pointer moves to the next word.
The loop continues until the end of the stream is reached.
{"heloo", "this"}
*/

std::vector<std::string> Server::split_cmd(std::string& cmd)
{
	std::vector<std::string> cmds_container;
	std::istringstream stream(cmd);
	std::string tokenized_cmd_with_spaces;

	while(stream >> tokenized_cmd_with_spaces){
		cmds_container.push_back(tokenized_cmd_with_spaces);
	}
	return cmds_container;
}
