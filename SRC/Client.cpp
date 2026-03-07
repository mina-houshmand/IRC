#include "../INC/Client.hpp"

Client::Client()
    : fd(-1)
    , isOperator(false)
    , registered(false)
    , logedin(false)
    , nickname("")
    , username("")
    , buffer("")
    , ipadd("")
    , ChannelsInvite()
{}
Client &Client::operator=(Client const &src){
	if (this != &src){
		this->nickname = src.nickname;
		this->username = src.username;
		this->fd = src.fd;
		this->ChannelsInvite = src.ChannelsInvite;
		this->buffer = src.buffer;
		this->registered = src.registered;
		this->ipadd = src.ipadd;
		this->logedin = src.logedin;
	}
	return *this;
}

void Client::set_IpAddress(struct in_addr addr){
    char* ipStr = inet_ntoa(addr);
    if (ipStr == NULL) {
        this->ipadd = "";
        throw std::runtime_error("Failed to convert IP address");
    }
    this->ipadd = ipStr;
}

void Client::addToBuffer(std::string recived_data){
	if (!recived_data.empty())
		buffer.append(recived_data);
}






//---------------//Getters
bool Client::GetInviteChannel(std::string &ChName){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == ChName)
			return true;
	}
	return false;
}
std::string Client::GetNickName(){return this->nickname;}
bool Client::GetLogedIn(){return this->logedin;}
std::string Client::GetUserName(){return this->username;}
std::string Client::getIpAdd(){return ipadd;}
std::string Client::getHostname(){
	std::string hostname = this->GetNickName() + "!" + this->GetUserName();
	return hostname;
}

void Client::SetClient_Fd(int fd){
	if (fd < 0) {
        throw std::invalid_argument("File descriptor cannot be negative");
    }
    this->fd = fd;
}





void Client::SetNickname(std::string& nickName){this->nickname = nickName;}
void Client::setLogedin(bool value){this->logedin = value;}
void Client::SetUsername(std::string& username){this->username = username;}
void Client::setRegistered(bool value){registered = value;}
//---------------//Setters
//---------------//Methods
void Client::clearBuffer(){buffer.clear();}
void Client::AddChannelInvite(std::string &chname){
	ChannelsInvite.push_back(chname);
}
void Client::RmChannelInvite(std::string &chname){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == chname)
			{this->ChannelsInvite.erase(this->ChannelsInvite.begin() + i); return;}
	}
}
//---------------//Methods