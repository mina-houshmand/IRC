#include "../INC/Server.hpp"
#include <iostream>

/*
JOIN <channel>{,<channel>} [<key>{,<key>}]

Rules:
|__ 1. Check for minimum parameters (JOIN + channel)
|__ 2. Check for JOIN 0 (leave all channels)
|__ 3. Tokenize channels and keys
|__ 4. Validate channel names (must start with # or &)
|__ 5. Check maximum channel constraint (max 10 channels)
|__ 6. Process each channel:
    |__ a. If exists, check permissions (key, invite-only, user limit), then join
    |__ b. If doesn't exist, create and make client admin
|__ 7. Announce join and send initial metadata (Topic, Names)
*/

void Server::NotifyJoin(Client *client, Channel &channel) 
{
    // 1. Send JOIN message to the client
    _sendResponse(RPL_JOINMSG(client->GetPrefix(), channel.GetName()), client->GetFd());

    // 2. Send the channel topic (if set)
    if (!channel.GetTopicName().empty()) 
    {
        _sendResponse(RPL_TOPICIS(client->GetNickName(), channel.GetName(), channel.GetTopicName()), client->GetFd());
    }

    // 3. Send the list of users in the channel
    _sendResponse(
        RPL_NAMREPLY(client->GetNickName(), channel.GetName(), channel.clientChannel_list()) +
        RPL_ENDOFNAMES(client->GetNickName(), channel.GetName()), 
        client->GetFd()
    );

    // 4. Notify all other users in the channel about the new join
    channel.sendTo_all(RPL_JOINMSG(client->GetPrefix(), channel.GetName()), client->GetFd());

    // 5. Bot welcome message for #trivia when a client joins
    if (channel.GetName() == "#trivia") {
        _sendResponse(
            ":trivia_bot!~bot@localhost PRIVMSG #trivia :Welcome to the TRIVIA! To start the trivia write PRIVMSG #trivia :start\r\n",
            client->GetFd()
        );
    }
}

int Server::numberOfChannelsThatJoined(const std::string &nickName) 
{
    int count = 0;
    for (size_t i = 0; i < this->channels.size(); i++) 
    {
        std::string tempNick = nickName;
        if (this->channels[i].clientInChannel(tempNick)) 
            count++;
    }
    return count;
}

void Server::CreateNewChannel(const std::pair<std::string, std::string> &channelKeyPair, int fd) 
{
    std::string channelName = channelKeyPair.first;
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string clientNick = client->GetNickName();

    // Check if the client is already in 10 channels
    if (numberOfChannelsThatJoined(clientNick) >= 10) 
    {
        _sendResponse(ERR_TOOMANYCHANNELS(clientNick), fd);
        return;
    }

    // Create the channel // Default max limit is handled automatically by Channel constructor/default
    Channel newChannel;
    newChannel.SetName(channelName);
    newChannel.add_admin(*client);
    newChannel.set_createiontime();
    
    this->channels.push_back(newChannel);

    // Get the actual reference to the new channel
    Channel &createdChannel = this->channels.back();

    std::cout << "\033[0;35m[JOIN]\033[0m -> \033[0;32m" << clientNick << "\033[0m created and joined \033[0;34m" << channelName << "\033[0m" << std::endl;

    // Notify the client
    NotifyJoin(client, createdChannel);
}

void Server::HandleExistingChannel(const std::pair<std::string, std::string> &channelKeyPair, size_t channelIndex, int fd) 
{
    const std::string &channelName = channelKeyPair.first;
    const std::string &key = channelKeyPair.second;
    Channel &channel = this->channels[channelIndex];
    Client *client = GetClient(fd);
    
    if (!client) 
        return;

    std::string clientNick = client->GetNickName();

    // 1. Check if the client is already in the channel
    if (channel.clientInChannel(clientNick)) 
        return;

    // 2. Validate channel password if the channel is password-protected
    if (channel.GetKey() == 1) 
    {
        if (key != channel.GetPassword()) 
        {
            _sendResponse(ERR_BADCHANNELKEY(clientNick, channelName), fd);
            return;
        }
    }

    // 3. Check if the channel is invite-only
    if (channel.GetInvitOnly() == 1) 
    {
        std::string channelNameCopy = channelName;
        if (!client->GetInviteChannel(channelNameCopy)) 
        {
            _sendResponse(ERR_INVITEONLYCHAN(clientNick, channelName), fd);
            return;
        }
        client->RmChannelInvite(channelNameCopy);
    }

    // 4. Check if the channel has a user limit
    if (channel.GetLimit() > 0) 
    {
        if (channel.GetClientsNumber() >= channel.GetLimit()) 
        {
            _sendResponse(ERR_CHANNELISFULL(clientNick, channelName), fd);
            return;
        }
    }

    // Check 10 channels limit before joining
    if (numberOfChannelsThatJoined(clientNick) >= 10) 
    {
        _sendResponse(ERR_TOOMANYCHANNELS(clientNick), fd);
        return;
    }

    // 5. All validations passed - add the client to the channel
    channel.add_client(*client);

    std::cout << "\033[0;35m[JOIN]\033[0m -> \033[0;32m" << clientNick << "\033[0m joined \033[0;34m" << channelName << "\033[0m" << std::endl;

    // 6. Notify the client and broadcast to all users in the channel
    NotifyJoin(client, channel);
}

void Server::ProcessJoinChannel(const std::pair<std::string, std::string> &channelKeyPair, int fd) 
{
    const std::string &channelName = channelKeyPair.first;

    // Check if the channel already exists
    for (size_t j = 0; j < this->channels.size(); j++) 
    {
        if (this->channels[j].GetName() == channelName) 
        {
            HandleExistingChannel(channelKeyPair, j, fd);
            return;
        }
    }

    // If the channel does not exist, create it
    CreateNewChannel(channelKeyPair, fd);
}

bool Server::TokenizeJoinCmd(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd)
{
    std::vector<std::string> parts = split_cmd(cmd);
    std::string buff;
    Client *client = GetClient(fd);
    
    if (!client)
        return false;

    std::string clientNick = client->GetNickName();

    // 1. Ensure minimum parameters (JOIN + channel)
    if (parts.size() < 2) 
    {
        token.clear();
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("JOIN")), fd);
        return false;
    }

    // Extract channels and keys
    std::string channel_Str = parts[1];
    std::string key_Str = (parts.size() > 2) ? parts[2] : "";

    // Split channels by commas
    std::istringstream chStream(channel_Str);
    while (std::getline(chStream, buff, ',')) 
    {
        token.push_back(std::make_pair(buff, ""));
    }

    // Split keys by commas and assign them to the corresponding channels
    if (!key_Str.empty()) 
    {
        std::istringstream keyStream(key_Str);
        size_t i = 0;
        while (std::getline(keyStream, buff, ',') && i < token.size()) 
        {
            token[i].second = buff;
            i++;
        }
    }

    // Validate channel names
    for (size_t i = 0; i < token.size(); i++) 
    {
        const std::string &channelName = token[i].first;

        // Ignore empty channel segments (e.g. #chan1,,#chan2)
        if (channelName.empty()) 
        {
            token.erase(token.begin() + i--);
            continue;
        }

        // Check if the channel name starts with # or &
        if (channelName[0] != '#' && channelName[0] != '&') 
        {
            _sendResponse(ERR_NOSUCHCHANNEL(clientNick, channelName), fd);
            token.erase(token.begin() + i--);
        }
    }

    return !token.empty();
}

void Server::JOIN(std::string cmd, int fd)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string clientNick = client->GetNickName();
    std::vector<std::string> parts = split_cmd(cmd);

    // 1. Check for JOIN 0 (Leave all channels)
    if (parts.size() >= 2 && parts[1] == "0") 
    {
        std::cout << "\033[0;35m[JOIN]\033[0m -> \033[0;32m" << clientNick << "\033[0m issued JOIN 0 (leaving all channels)" << std::endl;
        std::vector<std::string> channelsToLeave;
        
        for (size_t i = 0; i < this->channels.size(); i++) 
        {
            if (this->channels[i].get_client(fd) || this->channels[i].get_admin(fd)) 
            {
                channelsToLeave.push_back(this->channels[i].GetName());
            }
        }
        for (size_t i = 0; i < channelsToLeave.size(); i++) 
        {
            ProcessPartChannel(channelsToLeave[i], fd, "Left all channels");
        }
        return;
    }

    std::vector<std::pair<std::string, std::string> > token;
    
    // 2. Tokenize and validate command parameters
    if (!TokenizeJoinCmd(token, cmd, fd)) 
    {
        return;
    }

    // 3. Check hard limit of 10 channels processed simultaneously
    if (token.size() > 10) 
    {
        _sendResponse(ERR_TOOMANYTARGETS(clientNick), fd);
        return;
    }

    // 4. Process each validated channel token
    for (size_t i = 0; i < token.size(); i++) 
    {
        ProcessJoinChannel(token[i], fd);
    }
}
