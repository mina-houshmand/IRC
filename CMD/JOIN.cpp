#include "../INC/Server.hpp"

/*
IRC Rules to Check in ProcessJoinChannel
Channel Existence:

If the channel already exists, validate the client's ability to join it.
If the channel does not exist, create it.
Channel Membership:

If the client is already in the channel, do nothing.
Channel Password:

If the channel is password-protected, validate the provided key.
If the key is incorrect, return ERR_BADCHANNELKEY (475).
Invite-Only Channels:

If the channel is invite-only, check if the client is invited.
If the client is not invited, return ERR_INVITEONLYCHAN (473).
Channel User Limit:

If the channel has a user limit, ensure the limit is not exceeded.
If the limit is exceeded, return ERR_CHANNELISFULL (471).
Channel Creation:

If the channel does not exist, create it and make the client the operator.
Broadcast Join:

Notify all users in the channel that the client has joined.
Send the channel topic (if set) and the list of users to the joining client.
*/

void Server::NotifyJoin(Client *client, Channel &channel) {
    // Step 1: Send JOIN message to the client
    _sendResponse(
        RPL_JOINMSG(client->getHostname(), client->getIpAdd(), channel.GetName()),
        client->GetFd()
    );

    // Step 2: Send the channel topic (if set)
    if (!channel.GetTopicName().empty()) {
        _sendResponse(
            RPL_TOPICIS(client->GetNickName(), channel.GetName(), channel.GetTopicName()),
            client->GetFd()
        );
    }

    // Step 3: Send the list of users in the channel
    _sendResponse(
        RPL_NAMREPLY(client->GetNickName(), channel.GetName(), channel.clientChannel_list()) +
        RPL_ENDOFNAMES(client->GetNickName(), channel.GetName()),
        client->GetFd()
    );

    // Step 4: Notify all other users in the channel about the new join
    channel.sendTo_all(
        RPL_JOINMSG(client->getHostname(), client->getIpAdd(), channel.GetName()),
        client->GetFd()
    );
}

int Server::numberOfChannelsThatJoined(const std::string &nickName) {
	int count = 0;

    // Iterate through all channels and count the ones the client is a member of
    for (size_t i = 0; i < this->channels.size(); i++) {
        std::string tempNick = nickName;
        if (this->channels[i].clientInChannel(tempNick)) {
            count++;
        }
    }

    return count;
}

void Server::CreateNewChannel(const std::pair<std::string, std::string> &channelKeyPair, int fd) {
    const std::string &channelName = channelKeyPair.first;
    Client *client = GetClient(fd);

    // Step 1: Check if the client is already in 10 channels
    if (numberOfChannelsThatJoined(client->GetNickName()) >= 10) {
        senderror(405, client->GetNickName(), client->GetFd(), " :You have joined too many channels\r\n");
        return;
    }

    // Step 2: Create the channel
    Channel newChannel;
    newChannel.SetName(channelName); // Set the channel name
    newChannel.add_admin(*client);   // Make the client the channel operator
    newChannel.set_createiontime();  // Set the creation time
    this->channels.push_back(newChannel); // Add the channel to the server's list of channels

    // Step 3: Notify the client
    NotifyJoin(client, newChannel);
}

//check if the channel exists or not
void Server::ProcessJoinChannel(const std::pair<std::string, std::string> &channelKeyPair, int fd) {
    const std::string &channelName = channelKeyPair.first;
    const std::string &key = channelKeyPair.second;

    // Check if the channel already exists
    for (size_t j = 0; j < this->channels.size(); j++) {
        if (this->channels[j].GetName() == channelName) {
            HandleExistingChannel(channelKeyPair, j, fd);
            return;
        }
    }

    // If the channel does not exist, create it
    CreateNewChannel(channelKeyPair, fd);
}

// Process a single channel (either existing or new)
// void Server::ProcessJoinChannel(const std::pair<std::string, std::string> &channelKeyPair, int fd) {
//     const std::string &channelName = channelKeyPair.first;
//     const std::string &key = channelKeyPair.second;

// 	// Check if the channel already exists
//     for (size_t j = 0; j < this->channels.size(); j++) {
// 		bool flag = false;
//         if (this->channels[j].GetName() == channelName) {
//             ExistCh(channelKeyPair, j, fd);
// 			flag = true;
//             return;
//         }
//     }
// }

/*
EXAMPLE : JOIN #channel1,#channel2,#channel3 key1,key2,key3
Channels: #channel1, #channel2, #channel3
Keys: key1, key2, key3
The std::pair<std::string, std::string> is used to associate each channel with its corresponding key:

token[0] = {"#channel1", "key1"}
token[1] = {"#channel2", "key2"}
token[2] = {"#channel3", "key3"}
*/
bool Server::TokenizeJoinCmd(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd){
	std::vector<std::string> parts;
    std::istringstream iss(cmd);
    std::string buff;

	// Split the cmd into parts based on spaces and store them in the parts vector
	/*
	EXAMPLE:
	std::string cmd = "JOIN #channel1,#channel2 key1,key2";
	parts = {"JOIN", "#channel1,#channel2", "key1,key2"};
	*/
    while (iss >> buff) {
        parts.push_back(buff);
    }

	// Ensure there are enough parameters
	//join and one channal name is required at least(key is optioan)
    if (parts.size() < 2) {
        token.clear();
        senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n");
        return false;
    }

	// Extract channels and keys
    std::string channel_Str = parts[1];
	std::string key_Str = (parts.size() > 2) ? parts[2] : "";

	// Split channels by commas
	// getline function -> reads from the chStream stream until it encounters the delimiter , or the end of the string.
	// Each substring extracted is stored in the buff variable.
	// make pait to put the keys later
    std::istringstream chStream(channel_Str);
    while (std::getline(chStream, buff, ',')) {
        token.push_back(std::make_pair(buff, ""));
    }

	// Split keys by commas and assign them to the corresponding channels
	//we check the token.size() to ensure we don't assign more keys than there are channels.
	//JOIN #channel1 key1,key2 -> for this case we just ignore the extra keys
    if (!key_Str.empty()) {
        std::istringstream keyStream(key_Str);
        size_t i = 0;
        while (std::getline(keyStream, buff, ',') && i < token.size()) {
            token[i].second = buff;
            i++;
        }
    }

	// Validate channel names
    for (size_t i = 0; i < token.size(); i++) {
        const std::string &channelName = token[i].first;

        // Check for empty channel names
        if (channelName.empty()) {
            token.erase(token.begin() + i--);
            continue;
        }

        // Check if the channel name starts with # or &
        if (channelName[0] != '#' && channelName[0] != '&') {
            senderror(403, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->GetFd(), " :No such channel\r\n");
            token.erase(token.begin() + i--);  // Remove invalid channel from the token list
        }
    }

	// If no valid channels remain, return false
	if (token.empty()) {
		return false;
	}
	return true;
}


//cmd format : JOIN <channel>{,<channel>} [<key>{,<key>}]

/*
<channel>:

The name of the channel(s) to join.
		Multiple channels can be specified, separated by commas (,).
		Channel names must begin with # or & (e.g., #channel1).
<key> (optional):
		The password(s) for the channel(s), if required.
		Multiple keys can be specified, separated by commas (,), corresponding to the channels
*/

/*

Rules for the JOIN Command
Channel Name Validation:

Channel names must start with # or &.
If the channel name is invalid, respond with ERR_NOSUCHCHANNEL (403).
Channel Creation:

If the specified channel does not exist, it is created.
The client becomes the channel operator.
Channel Membership:

A client can only join up to 10 channels at a time.
If the client is already in 10 channels, respond with ERR_TOOMANYCHANNELS (405).
Channel Password:

If the channel is password-protected, the client must provide the correct password.
If the password is incorrect, respond with ERR_BADCHANNELKEY (475).
Invite-Only Channels:

If the channel is invite-only, the client must have an invitation to join.
If the client is not invited, respond with ERR_INVITEONLYCHAN (473).
Channel Limit:

If the channel has a user limit, the client cannot join if the limit is reached.
Respond with ERR_CHANNELISFULL (471).
Broadcast Join:

Notify all users in the channel that the client has joined.
Send the channel topic (if set) and the list of users in the channel to the joining client.
Error Handling:

If the JOIN command is missing required parameters, respond with ERR_NEEDMOREPARAMS (461).
If the client specifies more than 10 channels in a single JOIN command, respond with ERR_TOOMANYTARGETS (407).

*/
void Server::JOIN(std::string cmd, int fd)
{
	// Step 1: Parse the command into channels and keys
	/*
		token[0] = {"#channel1", "key1"};
		token[1] = {"#channel2", "key2"};
		token[2] = {"#channel3", "key3"};
	*/
	std::vector<std::pair<std::string, std::string> > token;
	if (!TokenizeJoinCmd(token, cmd, fd)) {
        senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n");
        return;
    }

	// Step 2: Check if the client is trying to join more than 10 channels
	if (token.size() > 10) {
		senderror(407, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Too many channels\r\n"); 
		return;
	}
	
	// Step 3: Process each channel
    for (size_t i = 0; i < token.size(); i++) {
        ProcessJoinChannel(token[i], fd);
    }
	
}