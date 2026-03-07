#include "../INC/Server.hpp"



void Server::handleClientCommand(std::string &cmd, int fd)
{
	if(cmd.empty())
		return ;
	
	//separates the command string into individual words/tokens based on whitespace
	std::vector<std::string> splited_cmd = split_cmd(cmd);

	//returns the index of the first character in cmd that is not one of the characters in the string " \t\v".
	size_t found = cmd.find_first_not_of(" \t\v");

	/*
	The substr method creates a substring of cmd starting from the index found.
	This removes all leading whitespace characters from cmd.
	*/
	if(found != std::string::npos)
		cmd = cmd.substr(found);

	//mina did this
	if (splited_cmd.empty())
		return;

	std::string command = splited_cmd[0];
	for (size_t i = 0; i < command.size(); ++i) {
		command[i] = std::toupper(command[i]);
	}	

	if (splited_cmd.size()){
		if(splited_cmd[0] == "BONG")
			return;
		if(splited_cmd[0] == "PASS" )
			client_authen(fd, cmd);
		else if (splited_cmd[0] == "NICK")
			set_nickname(cmd,fd);
		else if(splited_cmd[0] == "USER" )
			set_username(cmd, fd);
		else if (splited_cmd[0] == "QUIT")
			QUIT(cmd,fd);

		//The IRC protocol requires clients to send PASS, NICK, and USER commands to register with the server.
		//Only after successful registration can clients execute other commands.
		else if(isregistered(fd))
		{
			if (splited_cmd[0] == "KICK")
				KICK(cmd, fd);
			else if (splited_cmd[0] == "JOIN")
				JOIN(cmd, fd);
			else if (splited_cmd[0] == "TOPIC")
				Topic(cmd, fd);
			else if (splited_cmd[0] == "MODE")
				mode_command(cmd, fd);
			else if (splited_cmd[0] == "PART")
				PART(cmd, fd);
			else if (splited_cmd[0] == "PRIVMSG")
				PRIVMSG(cmd, fd);
			else if (splited_cmd[0] == "INVITE")
				Invite(cmd,fd);
			else if (splited_cmd.size())
				_sendResponse(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),splited_cmd[0]),fd);
		}
	}
	else if (!isregistered(fd))
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
