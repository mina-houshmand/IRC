#include "../INC/Server.hpp"

/*
-- Checks:
|__ 1. Command parameters (channel required)
|__ 2. Channel existence
|__ 3. Client membership in channel
|__ 4. Build part message with optional reason
|__ 5. Broadcast part to channel
|__ 6. Remove client from channel
|__ 7. Cleanup empty channel
*/

// Parse PART command into channels and optional reason
bool Server::SplitCmdPart(std::string cmd, std::vector<std::string> &tmp, std::string &reason, int fd)
{
    std::vector<std::string> parts = split_cmd(cmd);

    // Check for minimum parameters (PART + at least one channel)
    if (parts.size() < 2)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("PART")), fd);
        return false;
    }

    // Extract channel names (split by comma)
    std::string channel_Str = parts[1];
    std::string buff;
    std::istringstream chStream(channel_Str);

    while (std::getline(chStream, buff, ','))
    {
        if (!buff.empty())
            tmp.push_back(buff);
    }

    // Extract reason if provided (everything after channel list)
    size_t reasonPos = cmd.find(parts[1]);
    if (reasonPos != std::string::npos)
    {
        reasonPos = cmd.find_first_of(" :", reasonPos + parts[1].size());
        if (reasonPos != std::string::npos)
        {
            reason = cmd.substr(reasonPos);
            // Trim leading spaces and ensure ':' prefix
            size_t start = reason.find_first_not_of(" :");
            if (start != std::string::npos)
            {
                reason = reason.substr(start);
                if (!reason.empty() && reason[0] != ':')
                    reason = ":" + reason;
            }
        }
    }

    // Check if any valid channels remain
    if (tmp.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("PART")), fd);
        return false;
    }

    return true;
}

// Process parting from a single channel
void Server::ProcessPartChannel(const std::string &channelName, int fd, const std::string &reason)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string clientNick = client->GetNickName();
    std::string searchName = channelName;

    // Strip # or & prefix for comparison
    // if (!searchName.empty() && (searchName[0] == '#' || searchName[0] == '&'))
    //     ;

    // Find the channel in the server's channel list
    Channel *channel = GetChannel(searchName);

    // 2. Check if channel exists
    if (!channel)
    {
        _sendResponse(ERR_NOSUCHCHANNEL(clientNick, channelName), fd);
        return;
    }

    // 3. Check if client is in the channel
    if (!channel->get_client(fd) && !channel->get_admin(fd))
    {
        _sendResponse(ERR_NOTONCHANNEL(clientNick, searchName), fd);
        return;
    }

    // 4. Build part message with optional reason
    std::stringstream ss;
    ss << ":" << client->GetPrefix() << " PART " << channelName;
    if (!reason.empty())
        ss << " " << reason;
    ss << "\r\n";

    // DEBUG: Part broadcast
    std::cout << "\033[0;33m[PART]\033[0m -> " << "\033[0;36mChannel:\033[0m " << channelName 
              << " | " << "\033[0;36mLeaving:\033[0m " << clientNick 
              << " | " << "\033[0;33mSent:\033[0m " << ss.str() << std::endl;
    if (!reason.empty())
        std::cout << "\033[0;33m[PART]\033[0m -> " << "\033[0;36mReason:\033[0m " << reason << std::endl;

    // 5. Broadcast part message to all users in the channel
    channel->sendTo_all(ss.str());

    // 6. Remove client from channel (check if admin or regular client)
    if (channel->get_admin(fd))
        channel->remove_admin(fd);
    else
        channel->remove_client(fd);

    // 7. Delete channel if empty
    if (channel->GetClientsNumber() == 0)
        RemoveChannel(searchName);
}

/*
PART <channel>{,<channel>} [<reason>]

Rules:
|__ Channel names must start with # or &
|__ Client must be member of channel (ERR_NOTONCHANNEL if not)
|__ Broadcast PART message with optional reason
|__ Delete channel if empty after part
|__ ERR_NEEDMOREPARAMS if missing channel parameter
*/
void Server::PART(std::string cmd, int fd)
{
    std::vector<std::string> channels;
    std::string reason;

    // 1. Parse command into channels and optional reason
    if (!SplitCmdPart(cmd, channels, reason, fd))
        return;

    // 2. Process each channel
    for (size_t i = 0; i < channels.size(); i++)
    {
        ProcessPartChannel(channels[i], fd, reason);
    }
}
