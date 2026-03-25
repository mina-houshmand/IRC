#include "../INC/Server.hpp"

/*
-- Checks:
|__ 1. Parse command and extract channel name
|__ 2. Validate channel existence
|__ 3. Check client membership in channel
|__ 4. If no topic argument: return current topic
|__ 5. If setting topic: check operator privileges
|__ 6. Set new topic and broadcast to channel
*/

// Parse TOPIC command and extract topic text (everything after channel name)
std::string Server::ParseTopic(std::string &cmd)
{
    std::string topic = "";
    
    // Look for ':' which indicates the start of the topic text
    size_t colonPos = cmd.find(':');
    if (colonPos != std::string::npos)
    {
        topic = cmd.substr(colonPos + 1);
        // Remove trailing \r\n if present
        size_t endPos = topic.find("\r\n");
        if (endPos != std::string::npos)
            topic = topic.substr(0, endPos);
    }
    
    return topic;
}

/*
TOPIC <channel> [<topic>]

Rules:
|__ Channel must exist (ERR_NOSUCHCHANNEL if not)
|__ Client must be in channel (ERR_NOTONCHANNEL if not)
|__ If no topic: return current topic (RPL_TOPICIS or RPL_NOTOPIC)
|__ If setting topic: must be operator (ERR_CHANOPRIVSNEEDED if not)
|__ If topic_restriction is set: only operators can change topic
|__ Broadcast new topic to all channel members
*/
void Server::Topic(std::string &cmd, int &fd)
{
    std::vector<std::string> cmd_tokens = split_cmd(cmd);
    
    // 1. Check command parameters
    if (cmd_tokens.size() < 2)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(std::string("TOPIC")), fd);
        return;
    }
    
    std::string channelName = cmd_tokens[1];
    Client *client = GetClient(fd);
    if (!client)
        return;
    
    std::string clientNick = client->GetNickName();
    std::string searchName = channelName;
    
    // Strip # or & prefix for comparison
    // if (!searchName.empty() && (searchName[0] == '#' || searchName[0] == '&'))
    //     ;

    // 2. Find and validate channel existence
    Channel *channel = GetChannel(searchName);
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
    
    // 4. Check if topic argument is provided
    std::string newTopic = ParseTopic(cmd);
    
    // Check if this is a topic query (no topic argument)
    bool isQuery = (cmd_tokens.size() < 3 || 
                   (cmd_tokens.size() == 2) ||
                   (newTopic.empty() && cmd.find(':') == std::string::npos));
    
    if (isQuery)
    {
        // Return current topic
        std::string currentTopic = channel->GetTopicName();
        std::string chanName = channel->GetName();
        if (currentTopic.empty())
        {
            _sendResponse(RPL_NOTOPIC(clientNick, chanName), fd);
        }
        else
        {
            _sendResponse(RPL_TOPICIS(clientNick, chanName, currentTopic), fd);
        }
        return;
    }
    
    // 5. Setting new topic - check operator privileges if topic_restriction is set
    if (channel->Gettopic_restriction() && !channel->get_admin(fd))
    {
        _sendResponse(ERR_CHANOPRIVSNEEDED(clientNick, searchName), fd);
        return;
    }
    
    // 6. Set new topic and broadcast to channel
    channel->SetTopicName(newTopic);
    
    // Broadcast topic change to all channel members
    std::stringstream ss;
    ss << ":" << clientNick << "!~" << client->GetUserName() << "@localhost TOPIC " 
       << channelName << " :" << newTopic << "\r\n";
    
    // DEBUG: Topic change
    std::cout << "\033[0;34m[TOPIC]\033[0m -> " << "\033[0;36mChannel:\033[0m " << channelName 
              << " | " << "\033[0;36mSetBy:\033[0m " << clientNick 
              << " | " << "\033[0;34mSent:\033[0m " << ss.str() << std::endl;
    std::cout << "\033[0;34m[TOPIC]\033[0m -> " << "\033[0;36mTopic:\033[0m " << newTopic << std::endl;
    
    channel->sendTo_all(ss.str());
}
