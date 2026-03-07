#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include "Channel.hpp"
#include <arpa/inet.h>

class Client
{
private:
	int fd;
	bool isOperator;
	bool registered;
	bool logedin;
	std::string nickname;
	std::string username;
	std::string buffer;
	std::string ipadd;
	std::vector<std::string> ChannelsInvite;
public:
	Client();
	Client(std::string nickname, std::string username, int fd) :fd(fd), nickname(nickname), username(username){};
	~Client(){};
	Client(Client const &src){*this = src;};
	Client &operator=(Client const &src);


	void	SetClient_Fd(int fd);
	void	set_IpAddress(struct in_addr addr);
	int		GetFd(){return this->fd;}
	bool	getRegistered(){return registered;}


	//---------------//Getters

	bool GetInviteChannel(std::string &ChName);
	std::string GetNickName();
	bool 		GetLogedIn();
	std::string GetUserName();
	std::string getIpAdd();
	std::string getBuffer();
	std::string getHostname();

	void SetNickname(std::string& nickName);
	void setLogedin(bool value);
	void SetUsername(std::string& username);
	void setBuffer(std::string recived);
	void setRegistered(bool value);
	//---------------//Methods
	void clearBuffer();
	void AddChannelInvite(std::string &chname);
	void RmChannelInvite(std::string &chname);
};

#endif