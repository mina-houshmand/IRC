#include "../INC/Server.hpp"
#include <iostream>
#include <sstream>

/*
INVITE <nickname> <channel>

Rules:
|__ 1. Check for minimum parameters (INVITE + nickname + channel)
|__ 2. Channel must exist (ERR_NOSUCHCHANNEL)
|__ 3. Client must be on channel (ERR_NOTONCHANNEL)
|__ 4. Target user must exist (ERR_NOSUCHNICK)
|__ 5. Target user must not already be in the channel (ERR_USERONCHANNEL)
|__ 6. If channel is invite-only (+i), client must be an operator (ERR_CHANOPRIVSNEEDED)
|__ 7. Send RPL_INVITING to inviter
|__ 8. Send INVITE message to target
|__ 9. Add channel to target's invite list
*/

void Server::Invite(std::string &cmd, int &fd)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string clientNick = client->GetNickName();
    std::vector<std::string> parts = split_cmd(cmd);

    // 1. Check for minimum parameters (INVITE + nickname + channel)
    if (parts.size() < 3)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("INVITE")), fd);
        return;
    }

    std::string targetNick = parts[1];
    std::string channelName = parts[2];
    
    // Check if channel name is valid
    std::string searchName = channelName;

    // In IRC, channel names typically must start with # or &
    if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
    {
        _sendResponse(ERR_NOSUCHCHANNEL(clientNick, channelName), fd);
        return;
    }

    Channel *channel = GetChannel(searchName);

    // 2. Check if channel exists
    if (!channel)
    {
        _sendResponse(ERR_NOSUCHCHANNEL(clientNick, channelName), fd);
        return;
    }

    // 3. Check if inviting client is in the channel
    if (!channel->get_client(fd) && !channel->get_admin(fd))
    {
        _sendResponse(ERR_NOTONCHANNEL(clientNick, searchName), fd);
        return;
    }

    // 4. Check if target user exists
    Client *targetClient = GetClientNick(targetNick);
    if (!targetClient)
    {
        _sendResponse(ERR_NOSUCHNICK(clientNick, targetNick), fd);
        return;
    }

    // 5. Check if target user is already in the channel
    if (channel->GetClientInChannel(targetNick))
    {
        _sendResponse(ERR_USERONCHANNEL(clientNick, searchName), fd);
        return;
    }

    // 6. Check invite-only flag and operator status
    // If the channel carries the invite-only flag (+i), the user inviting MUST be a channel operator
    if (channel->GetInvitOnly() && !channel->get_admin(fd))
    {
        _sendResponse(ERR_CHANOPRIVSNEEDED(clientNick, channelName), fd);
        return;
    }

    // 9. Add channel to target's invite list
    targetClient->AddChannelInvite(searchName);

    // Print debug information in colorful style
    std::cout << "\033[0;35m[INVITE]\033[0m -> \033[0;32m" << clientNick << "\033[0m invited \033[0;31m" 
              << targetNick << "\033[0m to \033[0;34m" << channelName << "\033[0m" << std::endl;

    // 7. Send RPL_INVITING to inviter
    _sendResponse(RPL_INVITING(clientNick, targetNick, searchName), fd);

    // 8. Send INVITE message to target
    // Format: :<inviter_nick>!~<inviter_user>@<inviter_host> INVITE <target_nick> :<channel>
    std::string inviteMsg = ":" + client->GetPrefix() + 
                            " INVITE " + targetNick + " :" + channelName + "\r\n";
    _sendResponse(inviteMsg, targetClient->GetFd());
}

