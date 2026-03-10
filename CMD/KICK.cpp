#include "../INC/Server.hpp"

/*
-- Checks:
|__ 1. Command parameters (channel + user required)
|__ 2. Channel existence
|__ 3. Issuer membership in channel
|__ 4. Issuer operator privileges
|__ 5. Target user membership in channel
|__ 6. Broadcast kick message
|__ 7. Remove target from channel
|__ 8. Cleanup empty channel
*/

void	Server::KICK(std::string cmd, int fd)
{
	std::vector<std::string> cmd_tokens = split_cmd(cmd);

	// 1. Check command parameters
	if (cmd_tokens.size() < 3) {
		_sendResponse(ERR_NEEDMOREPARAMS(std::string("KICK")), fd);
		return;
	}

	std::string channelName = cmd_tokens[1];
	std::string targetUser = cmd_tokens[2];

	// Extract reason (everything after targetUser, with : prefix if not present)
	std::string reason = "";
	if (cmd_tokens.size() > 3) {
		size_t reasonPos = cmd.find(targetUser) + targetUser.size();
		reason = cmd.substr(reasonPos);
		// Trim leading spaces
		size_t start = reason.find_first_not_of(" \t");
		if (start != std::string::npos)
			reason = reason.substr(start);
		// Add : prefix if not present
		if (!reason.empty() && reason[0] != ':')
			reason = ":" + reason;
	}

	// 2. Check if channel exists
	Channel *channel = GetChannel(channelName);
	if (!channel) {
		_sendResponse(ERR_NOSUCHCHANNEL(GetClient(fd)->GetNickName(), channelName), fd);
		return;
	}

	// 3. Check if issuer is in the channel
	if (!channel->get_client(fd) && !channel->get_admin(fd)) {
		_sendResponse(ERR_KICKNOTONCHANNEL(GetClient(fd)->GetNickName(), channelName), fd);
		return;
	}

	// 4. Check if issuer is channel operator
	if (!channel->get_admin(fd)) {
		_sendResponse(ERR_KICKCHANOPRIVSNEEDED(GetClient(fd)->GetNickName(), channelName), fd);
		return;
	}

	// 5. Check if target user is in the channel
	Client *targetClient = channel->GetClientInChannel(targetUser);
	if (!targetClient) {
		_sendResponse(ERR_KICKUSERNOTINCHANNEL(GetClient(fd)->GetNickName(), channelName, targetUser), fd);
		return;
	}

	// 6. Broadcast kick message to all channel members
	std::stringstream ss;
	ss << ":" << GetClient(fd)->GetNickName() << "!~" << GetClient(fd)->GetUserName() 
	   << "@localhost KICK " << channelName << " " << targetUser;
	if (!reason.empty())
		ss << " " << reason;
	ss << "\r\n";
	
	// DEBUG: Kick broadcast
	std::cout << "\033[0;31m[KICK]\033[0m -> " << "\033[0;36mChannel:\033[0m " << channelName 
	          << " | " << "\033[0;36mKicked:\033[0m " << targetUser 
	          << " | " << "\033[0;36mBy:\033[0m " << GetClient(fd)->GetNickName() 
	          << " | " << "\033[0;31mSent:\033[0m " << ss.str() << std::endl;
	
	channel->sendTo_all(ss.str());

	// 7. Remove target user from channel
	int targetFd = targetClient->GetFd();
	if (channel->get_admin(targetFd))
		channel->remove_admin(targetFd);
	else
		channel->remove_client(targetFd);

	// 8. Remove channel if empty
	if (channel->GetClientsNumber() == 0)
		RemoveChannel(channelName);
}
