#include "../INC/Server.hpp"

/*
-- Checks:
|__ 1. Parse optional quit reason
|__ 2. Build quit message
|__ 3. Broadcast to all channels user is member of
|__ 4. Remove user from all channels
|__ 5. Cleanup empty channels
|__ 6. Close client connection
*/

// Parse QUIT command and extract optional reason
std::string Server::ParseQuitReason(std::string cmd)
{
    std::string reason = "";
    
    // Look for ':' which indicates the start of the reason
    size_t colonPos = cmd.find(':');
    if (colonPos != std::string::npos)
    {
        reason = cmd.substr(colonPos);
        // Ensure ':' prefix
        if (!reason.empty() && reason[0] != ':')
            reason = ":" + reason;
    }
    
    return reason;
}

// Build and broadcast quit message to all channels
void Server::BroadcastQuit(int fd, const std::string &reason)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string clientNick = client->GetNickName();
    std::string userName = client->GetUserName();

    // Build quit message
    std::stringstream ss;
    ss << ":" << clientNick << "!~" << userName << "@localhost QUIT";
    if (!reason.empty())
        ss << " " << reason;
    ss << "\r\n";

    std::string quitMsg = ss.str();

    // DEBUG: Quit broadcast
    std::cout << "\033[0;32m[QUIT]\033[0m -> " << "\033[0;36mUser:\033[0m " << clientNick 
              << " | " << "\033[0;32mSent:\033[0m " << quitMsg << std::endl;
    if (!reason.empty())
        std::cout << "\033[0;32m[QUIT]\033[0m -> " << "\033[0;36mReason:\033[0m " << reason << std::endl;

    // Send to all channels the user is a member of
    for (size_t i = 0; i < this->channels.size(); i++)
    {
        if (this->channels[i].get_client(fd) || this->channels[i].get_admin(fd))
        {
            this->channels[i].sendTo_all(quitMsg, fd);
        }
    }
}

/*
QUIT [<reason>]

Rules:
|__ Reason is optional
|__ Broadcast QUIT message to all channel members
|__ Remove user from all channels
|__ Delete empty channels
|__ Close client connection
*/
void Server::QUIT(std::string cmd, int fd)
{
    // 1. Parse optional quit reason
    std::string reason = ParseQuitReason(cmd);

    // 2-3. Build and broadcast quit message to all channels
    BroadcastQuit(fd, reason);

    // 4-6. Remove user from all channels, cleanup empty ones and close the connection
    disconnectClient(fd);
}
