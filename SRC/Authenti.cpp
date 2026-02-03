#include "../INC/Server.hpp"
#include <iostream>

/* 
*   PASS COMMAND
*/

void Server::client_authen(int fd, std::string cmd)
{
	Client *cli = GetClient(fd);

	std::cout << "Authenticating client <" << fd << "> with PASS command." << std::endl;
	//trim the word "pass" from the command
	cmd = cmd.substr(4);

	// Remove leading whitespace
	size_t pos = cmd.find_first_not_of("\t\v ");
	if(pos < cmd.size())
	{
		cmd = cmd.substr(pos);

		//erase the :
		if(cmd[0] == ':')
			cmd.erase(cmd.begin());
	}
	if(pos == std::string::npos) 
		_sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), fd);
	
	//if (cli->getRegistered() == false)
	else if(!cli->getRegistered())
	{
		std::string pass = cmd;

		/*here the "password" is the one we use it to start the server
		./irc <ip> <password>
		and the "pass" is the one the client send it to the server to connect
		*/
		if(pass == password){
			std::cout << "Client <" << fd << "> provided correct password." << std::endl;
			_sendResponse(RPL_CONNECTED(cli->GetNickName()), fd);
			cli->setRegistered(true);
		}
		else
            _sendResponse(ERR_INCORPASS(std::string("*")), fd);
	}
	else
        _sendResponse(ERR_ALREADYREGISTERED(GetClient(fd)->GetNickName()), fd);
}


/* 
*    NICK COMMAND
*/

static bool is_special_character(char c)
{
	if (c == '-' || c == '_' 
		|| c == '[' || c == ']' 
		|| c == '\\' || c == '^' 
		|| c == '{' || c == '}')
		return true;
	return false;
}

static bool valid_first_character(char c)
{
	if (std::isalpha(c) || is_special_character(c))
		return true;
	return false;
}

/*
std::isalnum(c):

This checks if the character c is alphanumeric, meaning it is either:
A letter (a-z, A-Z), or
A digit (0-9).
*/
static bool valid_nickname_characters(char c)
{
	if (std::isalnum(c) || is_special_character(c))
		return true;
	return false;
}

bool Server::is_validNickname(std::string& nickname)
{
		
	if(nickname.empty() || nickname.size() > 9)
		return false;
	if(!valid_first_character(nickname[0]))
		return false;
	for(size_t i = 1; i < nickname.size(); i++)
	{
		if (!valid_nickname_characters(nickname[i]))
			return false;
	}
	return true;
}
//to ensures that the nickname comparison is case-insensitive, which is required by the IRC protocol.
static std::string ft_toLower(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
		str[i] = std::tolower(str[i]);
	return str;
}

bool Server::nickNameInUse(std::string& nickname)
{
	std::string client_nickname;
	std::string lowercase_nickname = ft_toLower(nickname);

	for (size_t i = 0; i < this->clients.size(); i++)
	{
		client_nickname = ft_toLower(this->clients[i].GetNickName());
		if (client_nickname == lowercase_nickname)
			return true;
	}
	return false;
}


void Server::set_nickname(std::string cmd, int fd)
{
	Client *cli = GetClient(fd);
	std::string temo_nickname = "*";

	//trim the word "nick" from the command
	//trim the whithespace after nick
	cmd = cmd.substr(4);
	size_t pos = cmd.find_first_not_of("\t\v ");
	if(pos < cmd.size())
	{
		cmd = cmd.substr(pos);
		if(cmd[0] == ':')
			cmd.erase(cmd.begin());
	}

	if(pos == std::string::npos)
		{_sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), fd); return;}

	//check the nickname case insensitively
	std::string new_nickname = ft_toLower(cmd);
	std::string old_nickname = ft_toLower(cli->GetNickName());

	//if nick name is used we put * as a nickname temporary
	//to nsures that the client always has some identifier
	//otherwise it could cause issues in other commands that rely on the nickname	
	if (nickNameInUse(new_nickname) && new_nickname != old_nickname){
		if(cli->GetNickName().empty()){
			cli->SetNickname(temo_nickname);
		}

		//mina send  seperate errors for this two conditions!!!!!
	    _sendResponse(ERR_NICKINUSE(std::string(cmd)), fd); 
		return;
	}

	
	if(!is_validNickname(cmd)) {
		_sendResponse(ERR_ERRONEUSNICK(std::string(cmd)), fd);
		return;
	}

	//here we can change the nickname
	//if the nickname is valid and not used ...
	else
	{
		if(cli && cli->getRegistered())
		{
			std::string oldnick = cli->GetNickName();

			//change the nickname for that client
			_sendResponse(RPL_NICKCHANGE(oldnick, cmd), fd);
			cli->SetNickname(cmd);

			// Update the nickname in all channels
			//search for that client in all channels and change his nickname there
			for(size_t i = 0; i < channels.size(); i++){
				Client *cl = channels[i].GetClientInChannel(oldnick);
				if(cl)
					cl->SetNickname(cmd);
			}

			//
			if(!oldnick.empty() && ft_toLower(oldnick) != ft_toLower(cmd))
			{
			/*
			nickname "*" is used for clients who are not yet fully connected to server
			so after setting a nick name the client is fully logged in so we set it to true
			and notify client that he is connected
			and also notify other clients about the nickname change
			*/
				// Notify other clients about the nickname change
				if(oldnick == "*" && !cli->GetUserName().empty())
				{
					cli->setLogedin(true);
					_sendResponse(RPL_CONNECTED(cli->GetNickName()), fd);
					_sendResponse(RPL_NICKCHANGE(cli->GetNickName(),cmd), fd);
				}
				else
					_sendResponse(RPL_NICKCHANGE(oldnick,cmd), fd);
				return;
			}
			
		}
		//if the client is not registered
		else if (cli && !cli->getRegistered())
			_sendResponse(ERR_NOTREGISTERED(cmd), fd);
	}

	//check if the client is fully registered
	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*" && !cli->GetLogedIn())
	{
		cli->setLogedin(true);
		_sendResponse(RPL_CONNECTED(cli->GetNickName()), fd);
	}
}

/* 
    USER COMMAND
*/

static bool is_special_character_user(char c)
{
	if (c == '-' || c == '_' 
		|| c == '.' || c == '~')
		return true;
	return false;
}

static  bool valid_username_characters(char c)
{
	if (std::isalnum(c) || is_special_character_user(c))
		return true;
	return false;
}

static bool is_validUsername(std::string& username)
{
		
	if(username.empty() || username.size() > 9)
		return false;
	for(size_t i = 0; i < username.size(); i++)
	{
		if (!valid_username_characters(username[i]))
			return false;
	}
	return true;
}

//user format:
// USER <username> <hostname> <servername> <realname>
void	Server::set_username(std::string& cmd, int fd)
{
	std::vector<std::string> splited_cmd = split_cmd(cmd);
	std::cout << "Processing USER command from Client <" << fd << ">: " << cmd << std::endl;
	Client *cli = GetClient(fd); 

	//Parameters should be at least 5 (USER + 4 parameters)
	//<ho>
	std::cout << "Setting username for Client <" << fd << ">." << splited_cmd.size() << std::endl;
	if((cli && splited_cmd.size() < 5) || (!cli && splited_cmd.size() < 5)){
		_sendResponse(ERR_NOTENOUGHPARAM(cli->GetNickName()), fd); return; 
	}

	//if client does not exist or not registered
	if(!cli  || !cli->getRegistered()){
		_sendResponse(ERR_NOTREGISTERED(std::string("*")), fd);
	}

	//This prevents the client from changing their username after it has been set.
	//fix the error is wrong !!!!
	else if (cli && !cli->GetUserName().empty()){
		_sendResponse(ERR_ALREADYREGISTERED(cli->GetNickName()), fd); return;
	}
	else{
		if (is_validUsername(splited_cmd[1])){
			cli->SetUsername(splited_cmd[1]);
			_sendResponse(RPL_NAMECHANGE(cli->GetUserName(), splited_cmd[1], ""), fd);
		}else{
			_sendResponse(ERR_ERRONEUSUSERNAME(splited_cmd[1]), fd); return;
		}
	}
	//check if the client is fully registered
	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*"  && !cli->GetLogedIn()){
		cli->setLogedin(true);
		_sendResponse(RPL_CONNECTED(cli->GetNickName()), fd);
	}
}