#include "../INC/Server.hpp"

/*
-- Checks:
|__ 1. Parse channel name, mode string, and params
|__ 2. Channel existence
|__ 3. No modestring: return current channel modes
|__ 4. Client operator privileges
|__ 5. Process each mode character (+i -i +t -t +k -k +o -o +l -l)
|__ 6. Broadcast mode change to all channel members
*/

// Split mode parameter string by whitespace into a vector
std::vector<std::string>	Server::splitParams(std::string params)
{
	std::vector<std::string>	result;
	std::string					buff;
	std::istringstream			stream(params);

	while (stream >> buff)
		result.push_back(buff);
	return (result);
}

// Extract channel name, mode string, and extra params from MODE command
void	Server::getCmdArgs(std::string cmd, std::string &name,
			std::string &modeset, std::string &params)
{
	std::istringstream	stream(cmd);
	std::string			token;
	size_t				start;

	stream >> token;
	if (stream >> token)
		name = token;
	if (stream >> token)
		modeset = token;
	if (std::getline(stream, params))
	{
		start = params.find_first_not_of(" \t");
		if (start != std::string::npos)
			params = params.substr(start);
		else
			params = "";
	}
}

// Build the mode change string, appending operator and mode char
std::string	Server::mode_toAppend(std::string chain, char opera, char mode)
{
	std::string	modeChar(1, mode);

	if (chain.empty())
		return (std::string(1, opera) + modeChar);
	if (chain[0] != opera)
		return (chain + opera + modeChar);
	return (chain + modeChar);
}

// Validate that limit is a non-empty positive numeric string
bool	Server::isvalid_limit(std::string &limit)
{
	if (limit.empty())
		return (false);
	for (size_t i = 0; i < limit.size(); i++)
	{
		if (!std::isdigit(limit[i]))
			return (false);
	}
	return (true);
}

// Handle invite-only mode (+i / -i)
std::string	Server::invite_only(Channel *channel, char opera, std::string chain)
{
	if (opera == '+')
		channel->SetInvitOnly(1);
	else
		channel->SetInvitOnly(0);
	return (mode_toAppend(chain, opera, 'i'));
}

// Handle topic restriction mode (+t / -t)
std::string	Server::topic_restriction(Channel *channel, char opera,
				std::string chain)
{
	if (opera == '+')
		channel->set_topicRestriction(true);
	else
		channel->set_topicRestriction(false);
	return (mode_toAppend(chain, opera, 't'));
}

// Handle channel key/password mode (+k / -k)
std::string	Server::password_mode(std::vector<std::string> splited,
				Channel *channel, size_t &pos, char opera, int fd,
				std::string chain, std::string &arguments)
{
	if (opera == '+')
	{
		if (pos >= splited.size())
		{
			_sendResponse(ERR_NEEDMODEPARM(channel->GetName(), "(k)"), fd);
			return (chain);
		}
		if (channel->GetKey())
		{
			_sendResponse(ERR_KEYSET(channel->GetName()), fd);
			return (chain);
		}
		channel->SetPassword(splited[pos]);
		channel->SetKey(1);
		if (arguments.empty())
			arguments = splited[pos];
		else
			arguments += " " + splited[pos];
		chain = mode_toAppend(chain, opera, 'k');
		pos++;
	}
	else
	{
		channel->SetPassword("");
		channel->SetKey(0);
		chain = mode_toAppend(chain, opera, 'k');
	}
	return (chain);
}

// Handle operator privilege mode (+o / -o)
std::string	Server::operator_privilege(std::vector<std::string> splited,
				Channel *channel, size_t &pos, int fd, char opera,
				std::string chain, std::string &arguments)
{
	std::string	targetNick;
	Client		*target;

	if (pos >= splited.size())
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(), "(o)"), fd);
		return (chain);
	}
	targetNick = splited[pos];
	target = channel->GetClientInChannel(targetNick);
	if (!target)
	{
		_sendResponse(ERR_NOSUCHNICK(GetClient(fd)->GetNickName(), targetNick), fd);
		pos++;
		return (chain);
	}
	if (opera == '+')
	{
		if (channel->change_clientToAdmin(targetNick))
		{
			if (arguments.empty())
				arguments = targetNick;
			else
				arguments += " " + targetNick;
			chain = mode_toAppend(chain, opera, 'o');
		}
	}
	else
	{
		if (channel->change_adminToClient(targetNick))
		{
			if (arguments.empty())
				arguments = targetNick;
			else
				arguments += " " + targetNick;
			chain = mode_toAppend(chain, opera, 'o');
		}
	}
	pos++;
	return (chain);
}

// Handle channel user limit mode (+l / -l)
std::string	Server::channel_limit(std::vector<std::string> splited,
				Channel *channel, size_t &pos, char opera, int fd,
				std::string chain, std::string &arguments)
{
	std::string			limitStr;
	std::istringstream	ss;
	int					limitVal;

	if (opera == '+')
	{
		if (pos >= splited.size())
		{
			_sendResponse(ERR_NEEDMODEPARM(channel->GetName(), "(l)"), fd);
			return (chain);
		}
		limitStr = splited[pos];
		if (!isvalid_limit(limitStr))
		{
			_sendResponse(ERR_INVALIDMODEPARM(channel->GetName(), "(l)"), fd);
			pos++;
			return (chain);
		}
		ss.str(limitStr);
		ss >> limitVal;
		channel->SetLimit(limitVal);
		if (arguments.empty())
			arguments = limitStr;
		else
			arguments += " " + limitStr;
		chain = mode_toAppend(chain, opera, 'l');
		pos++;
	}
	else
	{
		channel->SetLimit(0);
		chain = mode_toAppend(chain, opera, 'l');
	}
	return (chain);
}

/*
MODE <channel> [<modestring>] [<arguments>]

Supported modes:
|__ +i / -i  Invite-only channel
|__ +t / -t  Topic settable by operators only
|__ +k / -k  Channel key (password)
|__ +o / -o  Grant / revoke operator privilege
|__ +l / -l  Set / unset channel user limit
*/
void	Server::mode_command(std::string &cmd, int fd)
{
	std::string				channelName;
	std::string				modeset;
	std::string				params;
	std::string				arguments;
	std::string				chain;
	std::string				searchName;
	Client					*client;
	Channel					*channel;
	char					opera;
	std::vector<std::string>	splited;
	size_t					pos;

	getCmdArgs(cmd, channelName, modeset, params);

	// Strip # or & prefix for channel lookup
	searchName = channelName;

	client = GetClient(fd);
	if (!client)
		return;

	// 2. Validate channel existence
	channel = GetChannel(searchName);
	if (!channel)
	{
		_sendResponse(ERR_CHANNELNOTFOUND(client->GetNickName(), channelName), fd);
		return;
	}

	// 3. No modestring: return current modes and creation time
	if (modeset.empty())
	{
		_sendResponse(RPL_CHANNELMODES(client->GetNickName(),
			channel->GetName(), channel->getModes()), fd);
		_sendResponse(RPL_CREATIONTIME(client->GetNickName(),
			channel->GetName(), channel->get_creationtime()), fd);
		return;
	}

	// 4. Check operator privileges
	if (!channel->get_admin(fd))
	{
		_sendResponse(ERR_NOTOPERATOR(channel->GetName()), fd);
		return;
	}

	// 5. Process each mode character
	opera = '+';
	splited = splitParams(params);
	pos = 0;
	for (size_t i = 0; i < modeset.size(); i++)
	{
		char c = modeset[i];
		if (c == '+' || c == '-')
			opera = c;
		else if (c == 'i')
			chain = invite_only(channel, opera, chain);
		else if (c == 't')
			chain = topic_restriction(channel, opera, chain);
		else if (c == 'k')
			chain = password_mode(splited, channel, pos, opera, fd, chain, arguments);
		else if (c == 'o')
			chain = operator_privilege(splited, channel, pos, fd, opera, chain, arguments);
		else if (c == 'l')
			chain = channel_limit(splited, channel, pos, opera, fd, chain, arguments);
		else
			_sendResponse(ERR_UNKNOWNMODE(client->GetNickName(),
				channel->GetName(), std::string(1, c)), fd);
	}

	// 6. Broadcast mode changes to all channel members
	if (!chain.empty())
	{
		std::string hostname = client->GetNickName() + "!~"
			+ client->GetUserName() + "@localhost";
		channel->sendTo_all(RPL_CHANGEMODE(hostname,
			channel->GetName(), chain, arguments));
	}
}
