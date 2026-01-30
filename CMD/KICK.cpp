#include "../INC/Server.hpp"

/*
Conditions to Check for KICK
Command Parameters:

The KICK command must include at least:
A valid channel name.
A valid user nickname to kick.
If these parameters are missing, respond with ERR_NEEDMOREPARAMS (461).
Channel Existence:

The specified channel must exist.
If the channel does not exist, respond with ERR_NOSUCHCHANNEL (403).
Client Membership:

The client issuing the KICK command must be a member of the channel.
If the client is not in the channel, respond with ERR_NOTONCHANNEL (442).
Operator Privileges:

The client issuing the KICK command must be a channel operator (admin).
If the client is not an operator, respond with ERR_CHANOPRIVSNEEDED (482).
Target User Membership:

The user to be kicked must be a member of the channel.
If the target user is not in the channel, respond with ERR_USERNOTINCHANNEL (441).
Broadcast the Kick:

Notify all users in the channel about the kick.
Include the optional reason (if provided) in the notification.
Channel Cleanup:

Remove the kicked user from the channel.
If the channel becomes empty, delete the channel.
*/


void	Server::KICK(std::string cmd, int fd)
{
	/*1-Command Parameters:
		The KICK command must include at least:
			A valid channel name.
			A valid user nickname to kick.
	*/
	std::vector<std::string> cmd_tokens = split_cmd(cmd);
	// Ensure we have at least <channel>, <user>, and optional <reason>
    if (cmd_tokens.size() < 3) { 
        senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n");
        return;
    }

	//2- channel Existence:

	std::string channelName = cmd_tokens[1];
    std::string targetUser = cmd_tokens[2];
	/*
	cmd.find(targetUser) finds the position of the target user in the command string.
	targetUser.size() adds the length of the target user's nickname to get the position just after the nickname.
	Adding + 1 skips the space after the nickname.
	*/
    std::string reason = (cmd_tokens.size() > 3) ? cmd.substr(cmd.find(targetUser) + targetUser.size() + 1) : ":No reason provided";
    // Ensure the reason starts with ':'
    if (!reason.empty() && reason[0] != ':') {
        reason.insert(reason.begin(), ':');
    }

    // Check if the channel exists
    Channel *channel = GetChannel(channelName);
    if (!channel) {
        senderror(403, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->GetFd(), " :No such channel\r\n");
        //OR
		//senderror(403, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :No such channel\r\n");
		return;
    }

	// Check if the client issuing the command is in the channel
    if (!channel->get_client(fd) && !channel->get_admin(fd)) {
        senderror(442, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->GetFd(), " :You're not on that channel\r\n");
        return;
    }

    // Check if the client issuing the command is an admin
    if (!channel->get_admin(fd)) {
        senderror(482, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->GetFd(), " :You're not channel operator\r\n");
        //OR 
		//{senderror(482, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :You're not channel operator\r\n"); continue;}

		return;
    }

	// Check if the target user is in the channel
    if (!channel->GetClientInChannel(targetUser)) {
        senderror(441, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->GetFd(), " :They aren't on that channel\r\n");
		// OR
		//{senderror(441, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :They aren't on that channel\r\n"); continue;}

        return;
    }

	/*until here we checked if :
	1- the channel is valid
	2- the client has the right to kick
	3-and the other client is in the channel
	*/

	// Broadcast the kick to all users in the channel
    std::stringstream ss;
    ss << ":" << GetClient(fd)->GetNickName() << "!~" << GetClient(fd)->GetUserName() << "@localhost KICK " << channelName << " " << targetUser << " " << reason << "\r\n";
	if (!reason.empty())
		ss << " :" << reason << "\r\n";
	channel->sendTo_all(ss.str());

	// Remove the target user from the channel
	/* 			channel->GetClientInChannel(targetUser)
	here we pass the target users name and find if it is in the channal and it returns the 
	clients object and then we get its fd 
	and then pass that fd to the get_admin to check if it is admin or client
	then remove it
	*/
	if (channel->get_admin(channel->GetClientInChannel(targetUser)->GetFd()))
		channel->remove_admin(channel->GetClientInChannel(targetUser)->GetFd());
	else
		channel->remove_client(channel->GetClientInChannel(targetUser)->GetFd());

    // If the channel is empty, delete it
    if (channel->GetClientsNumber() == 0) {
		RemoveChannel(channelName);

	}

}
