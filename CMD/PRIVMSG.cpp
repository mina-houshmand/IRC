#include "../INC/Server.hpp"

/*
-- Checks:
|__ 1. Command parameters (target + message required)
|__ 2. Parse targets and message
|__ 3. Check if target is channel or nickname
|__ 4. For channels: validate existence and membership
|__ 5. For users: validate existence
|__ 6. Broadcast message to target(s)
*/

// Parse PRIVMSG command into targets and message
// Returns false if not enough parameters
bool Server::SplitPrivMsg(std::string cmd, std::vector<std::string> &targets, std::string &message, int fd)
{
    std::vector<std::string> parts = split_cmd(cmd);

    // Check for minimum parameters (PRIVMSG + target + message)
    if (parts.size() < 3)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("PRIVMSG")), fd);
        return false;
    }

    // Extract targets (split by comma)
    std::string target_Str = parts[1];
    std::string buff;
    std::istringstream chStream(target_Str);

    while (std::getline(chStream, buff, ','))
    {
        if (!buff.empty())
            targets.push_back(buff);
    }

    // Extract message (everything after target, with : prefix)
    // Message format: :message or just message text
    size_t msgPos = cmd.find(parts[1]);
    if (msgPos != std::string::npos)
    {
        msgPos = cmd.find(':', msgPos);
        if (msgPos != std::string::npos)
        {
            message = cmd.substr(msgPos);
            // Ensure ':' prefix
            if (!message.empty() && message[0] != ':')
                message = ":" + message;
        }
    }

    // Check if any valid targets remain
    if (targets.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("PRIVMSG")), fd);
        return false;
    }

    return true;
}

// Send message to a channel
// Validates channel existence and client membership
void Server::SendToChannel(const std::string &channelName, int fd, const std::string &message)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string clientNick = client->GetNickName();
    std::string searchName = channelName;

    // Strip # or & prefix for comparison
    if (!searchName.empty() && (searchName[0] == '#' || searchName[0] == '&'))
        searchName = searchName.substr(1);

    // Find the channel
    Channel *channel = GetChannel(searchName);

    // Check if channel exists
    if (!channel)
    {
        _sendResponse(ERR_NOSUCHCHANNEL(clientNick, channelName), fd);
        return;
    }

    // Check if client is in the channel
    if (!channel->get_client(fd) && !channel->get_admin(fd))
    {
        _sendResponse(ERR_CANNOTSENDTOCHAN(clientNick, searchName), fd);
        return;
    }

    // Build and broadcast message
    std::stringstream ss;
    ss << ":" << clientNick << "!~" << client->GetUserName() << "@localhost PRIVMSG " << channelName << " " << message << "\r\n";
    
    // DEBUG: Channel message broadcast
    std::cout << "\033[0;35m[PRIVMSG-CHAN]\033[0m -> " << "\033[0;36mChannel:\033[0m " << channelName 
              << " | " << "\033[0;36mFrom:\033[0m " << clientNick 
              << " | " << "\033[0;35mSent:\033[0m " << ss.str() << std::endl;
    
    channel->sendTo_all(ss.str(), fd);
}

// Send message to a user
// Validates user existence
void Server::SendToUser(const std::string &nickname, int fd, const std::string &message)
{
    Client *sender = GetClient(fd);
    if (!sender)
        return;

    std::string senderNick = sender->GetNickName();

    // Find the target user
    Client *target = GetClientNick(nickname);

    // Check if user exists
    if (!target)
    {
        _sendResponse(ERR_NOSUCHNICK(senderNick, nickname), fd);
        return;
    }

    // Check if sending to self
    if (target->GetFd() == fd)
    {
        return;
    }

    // Build and send message
    std::stringstream ss;
    ss << ":" << senderNick << "!~" << sender->GetUserName() << "@localhost PRIVMSG " << nickname << " " << message << "\r\n";
    
    // DEBUG: Direct message send
    std::cout << "\033[0;35m[PRIVMSG-DM]\033[0m -> " << "\033[0;36mFrom:\033[0m " << senderNick 
              << " -> " << "\033[0;36mTo:\033[0m " << nickname << " (fd:" << target->GetFd() << ")" 
              << " | " << "\033[0;35mSent:\033[0m " << ss.str() << std::endl;
    
    _sendResponse(ss.str(), target->GetFd());
}

/*
PRIVMSG <target>{,<target>} :<message>

Rules:
|__ Target must be valid channel or nickname
|__ For channels: must be member (ERR_CANNOTSENDTOCHAN if not)
|__ For users: must exist (ERR_NOSUCHNICK if not found)
|__ Message must be provided with : prefix
|__ Broadcast to all channel members except sender
|__ ERR_NEEDMOREPARAMS if missing target or message
*/
void Server::PRIVMSG(std::string cmd, int fd)
{
    std::vector<std::string> targets;
    std::string message;

    // 1. Parse command into targets and message
    if (!SplitPrivMsg(cmd, targets, message, fd))
        return;

    // 2. Process each target
    for (size_t i = 0; i < targets.size(); i++)
    {
        std::string target = targets[i];

        // 3. Check if target is channel or nickname
        if (!target.empty() && (target[0] == '#' || target[0] == '&'))
        {
            // 4-5. Send to channel
            SendToChannel(target, fd, message);
        }
        else
        {
            // 5. Send to user
            SendToUser(target, fd, message);
        }
    }
}
