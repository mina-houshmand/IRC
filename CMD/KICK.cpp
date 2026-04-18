#include "../INC/Server.hpp"
#include <iostream>
#include <sstream>

/*
KICK <channel> <user> *( "," <user> ) [<comment>]

Rules:
|__ 1. Channel must exist (ERR_NOSUCHCHANNEL)
|__ 2. Client must be on channel (ERR_NOTONCHANNEL)
|__ 3. Client must be operator (ERR_CHANOPRIVSNEEDED)
|__ 4. Target must be on channel (ERR_USERNOTINCHANNEL)
|__ 5. Broadcast KICK message to all channel members
|__ 6. Remove target from channel
*/

void Server::KICK(std::string cmd, int fd)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string clientNick = client->GetNickName();
    std::vector<std::string> parts = split_cmd(cmd);

    // 1. Check for minimum parameters (KICK + channel + user)
    if (parts.size() < 3)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("KICK")), fd);
        return;
    }

    std::string channelName = parts[1];
    std::string targetNicks = parts[2];
    std::string reason = "";

    // Extract reason if provided
    size_t reasonPos = cmd.find(parts[2]);
    if (reasonPos != std::string::npos)
    {
        reasonPos = cmd.find_first_not_of(" \t", reasonPos + parts[2].length());
        if (reasonPos != std::string::npos)
        {
            reason = cmd.substr(reasonPos);
            if (reason[0] == ':')
                reason = reason.substr(1);
        }
    }

    // Strip # or & prefix for internal operations
    std::string searchName = channelName;

    Channel *channel = GetChannel(searchName);

    // 2. Check if channel exists
    if (!channel)
    {
        _sendResponse(ERR_NOSUCHCHANNEL(clientNick, channelName), fd);
        return;
    }

    // 3. Check if kicking client is in the channel
    if (!channel->get_client(fd) && !channel->get_admin(fd))
    {
        _sendResponse(ERR_KICKNOTONCHANNEL(clientNick, channelName), fd);
        return;
    }

    // 4. Check if kicking client is an operator
    if (!channel->get_admin(fd))
    {
        _sendResponse(ERR_KICKCHANOPRIVSNEEDED(clientNick, channelName), fd);
        return;
    }

    // Process each target individually
    std::istringstream targetStream(targetNicks);
    std::string target;
    while (std::getline(targetStream, target, ','))
    {
        if (target.empty())
            continue;

        // 5. Check if target user is in the channel
        Client *targetClient = channel->GetClientInChannel(target);
        if (!targetClient)
        {
            _sendResponse(ERR_KICKUSERNOTINCHANNEL(clientNick, channelName, target), fd);
            continue;
        }

        std::string finalReason = reason.empty() ? target : reason;

        // 6. Build the KICK message
        // Format: :<kicker>!~<user>@<host> KICK <channel> <target> :<reason>
        std::stringstream ss;
        ss << ":" << client->GetPrefix() 
           << " KICK " << channelName << " " << target << " :" << finalReason << "\r\n";
        
        std::string kickMsg = ss.str();

        // Print debug information in colorful style
        std::cout << "\033[0;35m[KICK]\033[0m -> \033[0;32m" << clientNick << "\033[0m kicked \033[0;31m" 
                  << target << "\033[0m from \033[0;34m" << channelName << "\033[0m "
                  << "(\033[0;36mReason:\033[0m " << finalReason << ")" << std::endl;

        // 7. Broadcast kick message to all users in the channel (including the target before they are removed)
        channel->sendTo_all(kickMsg);

        // 8. Remove target from channel
        if (channel->get_admin(targetClient->GetFd()))
            channel->remove_admin(targetClient->GetFd());
        else
            channel->remove_client(targetClient->GetFd());
    }

    // 9. Delete channel if it became empty (e.g., operator kicks themselves)
    if (channel->GetClientsNumber() == 0)
    {
        RemoveChannel(searchName);
        std::cout << "\033[0;33m[CHANNEL]\033[0m -> Channel \033[0;34m#" << searchName << "\033[0m deleted (empty)" << std::endl;
    }
}
