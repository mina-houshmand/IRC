#include "../INC/Server.hpp"

std::string extractQuitReason(const std::string &cmd) {

	std::string reason =cmd.substr(4);

	// Remove leading whitespace
	size_t pos = reason.find_first_not_of("\t\v ");
	if(pos != std::string::npos)
	{
		reason = reason.substr(pos);
	}else{
		reason.clear();
	}
	
	// If the reason doesn't start with ':', add it
    if (!reason.empty() && reason[0] != ':') {
        reason.insert(reason.begin(), ':');
    }

	// Default reason if none is provided
    if (reason.empty()) {
        reason = ":Quit";
    }
	
	return reason;
}



void Server::QUIT(std::string cmd, int fd)
{

	std::string reason = extractQuitReason(cmd);

	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].get_client(fd)){
			channels[i].remove_client(fd);
			//i????
			if (channels[i].GetClientsNumber() == 0)
				channels.erase(channels.begin() + i);

			else{ //notify other clients in the channel that this client has quit
				std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT " + reason + "\r\n";
				channels[i].sendTo_all(rpl);
			}
		}
		else if (channels[i].get_admin(fd)){
			channels[i].remove_admin(fd);
			if (channels[i].GetClientsNumber() == 0)
				channels.erase(channels.begin() + i);
			else{
				std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT " + reason + "\r\n";
				channels[i].sendTo_all(rpl);
			}
		}
	}
	std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
	RmChannels(fd);
	RemoveClient(fd);
	RemoveFds(fd);
	close(fd);
}