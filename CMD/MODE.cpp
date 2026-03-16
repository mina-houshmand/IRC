#include "../INC/Server.hpp"

/*
-- Helper Functions:
|__ mode_toAppend()  - Build mode string with +/- prefix
|__ getCmdArgs()     - Parse channel name, modeset, and params from command
|__ splitParams()    - Split comma-separated parameters into tokens

-- Checks (mode_command):
|__ 1. Parse command and extract arguments
|__ 2. Validate channel name and existence
|__ 3. Check issuer membership in channel
|__ 4. Handle mode query (no modeset provided)
|__ 5. Check operator privileges
|__ 6. Process each mode character (i, t, k, o, l)
|__ 7. Broadcast mode changes to channel
*/

// Builds mode string with proper +/- prefix
std::string Server::mode_toAppend(std::string chain, char opera, char mode)
{
	std::stringstream ss;
	char last = '\0';
	for(size_t i = 0; i < chain.size(); i++)
	{
		if(chain[i] == '+' || chain[i] == '-')
			last = chain[i];
	}
	if(last != opera)
		ss << opera << mode;
	else
		ss << mode;
	return ss.str();
}

// Parses channel name, modeset, and parameters from command string
void Server::getCmdArgs(std::string cmd, std::string& name, std::string& modeset, std::string &params)
{
	std::istringstream stm(cmd);
	stm >> name;
	stm >> modeset;
	size_t found = cmd.find_first_not_of(name + modeset + " \t\v");
	if(found != std::string::npos)
		params = cmd.substr(found);
}

// Splits comma-separated parameters into individual tokens
std::vector<std::string> Server::splitParams(std::string params)
{
	if(!params.empty() && params[0] == ':')
		params.erase(params.begin());
	std::vector<std::string> tokens;
	std::string param;
	std::istringstream stm(params);
	while (std::getline(stm, param, ','))
	{
		tokens.push_back(param);
		param.clear();
	}
	return tokens;
}

// Main MODE command handler
void Server::mode_command(std::string& cmd, int fd)
{
	std::string channelName;
	std::string params;
	std::string modeset;
	std::stringstream mode_chain;
	std::string arguments = ":";
	Channel *channel;
	char opera = '\0';

	arguments.clear();
	mode_chain.clear();
	Client *cli = GetClient(fd);
	
	// 1. Parse command - extract everything after "MODE"
	size_t found = cmd.find_first_not_of("MODEmode \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	else
	{
		_sendResponse(ERR_NOTENOUGHPARAM(cli->GetNickName()), fd);
		return ;
	}
	
	// 1. Extract channel name, modeset, and parameters
	getCmdArgs(cmd ,channelName, modeset ,params);
	std::vector<std::string> tokens = splitParams(params);
	
	// 2. Validate channel name and existence
	if(channelName[0] != '#' || !(channel = GetChannel(channelName.substr(1))))
	{
		_sendResponse(ERR_CHANNELNOTFOUND(cli->GetUserName(),channelName), fd);
		return ;
	}
	// 3. Check if issuer is in the channel
	else if (!channel->get_client(fd) && !channel->get_admin(fd))
	{
		senderror(442, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->GetFd(), " :You're not on that channel\r\n"); return;
	}
	// 4. Handle mode query (no modeset provided) - return current modes
	else if (modeset.empty())
	{
		_sendResponse(RPL_CHANNELMODES(cli->GetNickName(), channel->GetName(), channel->getModes()) + \
		RPL_CREATIONTIME(cli->GetNickName(), channel->GetName(),channel->get_creationtime()),fd);
		return ;
	}
	// 5. Check if issuer is channel operator
	else if (!channel->get_admin(fd))
	{
		_sendResponse(ERR_NOTOPERATOR(channel->GetName()), fd);
		return ;
	}
	// 6. Process each mode character
	else if(channel)
	{
		size_t pos = 0;
		for(size_t i = 0; i < modeset.size(); i++)
		{
			if(modeset[i] == '+' || modeset[i] == '-')
				opera = modeset[i];
			else
			{
				if(modeset[i] == 'i') // invite mode
					mode_chain << invite_only(channel , opera, mode_chain.str());
				else if (modeset[i] == 't') // topic restriction mode
					mode_chain << topic_restriction(channel, opera, mode_chain.str());
				else if (modeset[i] == 'k') // password set/remove
					mode_chain <<  password_mode(tokens, channel, pos, opera, fd, mode_chain.str(), arguments);
				else if (modeset[i] == 'o') // set/remove user operator privilege
						mode_chain << operator_privilege(tokens, channel, pos, fd, opera, mode_chain.str(), arguments);
				else if (modeset[i] == 'l') // set/remove channel limits
					mode_chain << channel_limit(tokens, channel, pos, opera, fd, mode_chain.str(), arguments);
				else
					_sendResponse(ERR_UNKNOWNMODE(cli->GetNickName(), channel->GetName(),modeset[i]),fd);
			}
		}
	}
	// 7. Broadcast mode changes to channel
	std::string chain = mode_chain.str();
	if(chain.empty())
		return;

	// Build mode response
	std::string modeMsg = RPL_CHANGEMODE(cli->getHostname(), channel->GetName(), mode_chain.str(), arguments);
	
	// DEBUG: Mode change
	std::cout << "\033[0;35m[MODE]\033[0m -> " << "\033[0;36mChannel:\033[0m " << channel->GetName() 
	          << " | " << "\033[0;36mBy:\033[0m " << cli->GetNickName() 
	          << " | " << "\033[0;36mModes:\033[0m " << chain;
	if (!arguments.empty())
		std::cout << " | " << "\033[0;36mArgs:\033[0m " << arguments;
	std::cout << " | " << "\033[0;35mSent:\033[0m " << modeMsg << std::endl;

	channel->sendTo_all(modeMsg);
}

// Toggle invite-only mode (+i/-i)
std::string Server::invite_only(Channel *channel, char opera, std::string chain)
{
	std::string param;
	param.clear();
	if(opera == '+' && !channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, true);
		channel->SetInvitOnly(1);
		param =  mode_toAppend(chain, opera, 'i');
	}
	else if (opera == '-' && channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, false);
		channel->SetInvitOnly(0);
		param =  mode_toAppend(chain, opera, 'i');
	}
	return param;
}

// Toggle topic restriction mode (+t/-t) - only ops can change topic
std::string Server::topic_restriction(Channel *channel, char opera, std::string chain)
{
	std::string param;
	param.clear();
	if(opera == '+' && !channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, true);
		channel->set_topicRestriction(true);
		param =  mode_toAppend(chain, opera, 't');
	}
	else if (opera == '-' && channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, false);
		channel->set_topicRestriction(false);
		param =  mode_toAppend(chain, opera, 't');
	}
	return param;
}

// Validate password format (alphanumeric + underscore only)
bool validPassword(std::string password)
{
	if(password.empty())
		return false;
	for(size_t i = 0; i < password.size(); i++)
	{
		if(!std::isalnum(password[i]) && password[i] != '_')
			return false;
	}
	return true;
}

// Set/remove channel key (+k/-k)
std::string Server::password_mode(std::vector<std::string> tokens, Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string &arguments)
{
	std::string param;
	std::string pass;

	param.clear();
	pass.clear();
	// Check if parameter is provided
	if(tokens.size() > pos)
		pass = tokens[pos++];
	else
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(),std::string("(k)")),fd);
		return param;
	}
	// Validate password format
	if(!validPassword(pass))
	{
		_sendResponse(ERR_INVALIDMODEPARM(channel->GetName(),std::string("(k)")),fd);
		return param;
	}
	// Set channel key
	if(opera == '+')
	{
		channel->setModeAtindex(2, true);
		channel->SetPassword(pass);
		if(!arguments.empty())
			arguments += " ";
		arguments += pass;
		param = mode_toAppend(chain,opera, 'k');
	}
	// Remove channel key
	else if (opera == '-' && channel->getModeAtindex(2))
	{
		if(pass == channel->GetPassword())
		{
			channel->setModeAtindex(2, false);
			channel->SetPassword("");
			param = mode_toAppend(chain,opera, 'k');
		}
		else
			_sendResponse(ERR_KEYSET(channel->GetName()),fd);
	}
	return param;
}

// Set/remove operator privileges (+o/-o)
std::string Server::operator_privilege(std::vector<std::string> tokens, Channel *channel, size_t& pos, int fd, char opera, std::string chain, std::string& arguments)
{
	std::string user;
	std::string param;

	param.clear();
	user.clear();
	// Check if user parameter is provided
	if(tokens.size() > pos)
		user = tokens[pos++];
	else
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(),"(o)"),fd);
		return param;
	}
	// Check if user is in channel
	if(!channel->clientInChannel(user))
	{
		_sendResponse(ERR_NOSUCHNICK(channel->GetName(), user),fd);
		return param;
	}
	// Grant operator status
	if(opera == '+')
	{
		channel->setModeAtindex(3,true);
		if(channel->change_clientToAdmin(user))
		{
			param = mode_toAppend(chain, opera, 'o');
			if(!arguments.empty())
				arguments += " ";
			arguments += user;
		}
	}
	// Remove operator status
	else if (opera == '-')
	{
		channel->setModeAtindex(3,false);
		if(channel->change_adminToClient(user))
		{
			param = mode_toAppend(chain, opera, 'o');
			if(!arguments.empty())
				arguments += " ";
			arguments += user;
		}
	}
	return param;
}

// Validate channel limit format (positive integer only)
bool Server::isvalid_limit(std::string& limit)
{
	return (!(limit.find_first_not_of("0123456789")!= std::string::npos) && std::atoi(limit.c_str()) > 0);
}

// Set/remove channel user limit (+l/-l)
std::string Server::channel_limit(std::vector<std::string> tokens,  Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string& arguments)
{
	std::string limit;
	std::string param;

	param.clear();
	limit.clear();
	if(opera == '+')
	{
		// Check if limit parameter is provided
		if(tokens.size() > pos )
		{
			limit = tokens[pos++];
			// Validate limit format
			if(!isvalid_limit(limit))
			{
				_sendResponse(ERR_INVALIDMODEPARM(channel->GetName(),"(l)"), fd);
			}
			else
			{
				channel->setModeAtindex(4, true);
				channel->SetLimit(std::atoi(limit.c_str()));
				if(!arguments.empty())
					arguments += " ";
				arguments += limit;
				param =  mode_toAppend(chain, opera, 'l');
			}
		}
		else
			_sendResponse(ERR_NEEDMODEPARM(channel->GetName(),"(l)"),fd);
	}
	// Remove channel limit
	else if (opera == '-' && channel->getModeAtindex(4))
	{
		channel->setModeAtindex(4, false);
		channel->SetLimit(0);
		param  = mode_toAppend(chain, opera, 'l');
	}
	return param;
}