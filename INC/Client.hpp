#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <arpa/inet.h>
#include <string>
#include <vector>

class Server;
class Channel;

class Client
{
private:
	int fd;
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


	void		SetClient_Fd(int fd);
	void		set_IpAddress(struct in_addr addr);
	int			GetFd(){return this->fd;}
	bool		getRegistered(){return registered;}
	std::string GetCmds(){return buffer;}



	bool		GetInviteChannel(std::string &ChName);
	std::string GetNickName(){return this->nickname;}
	std::string GetUserName(){return this->username;}
	bool 		GetLogedIn(){return this->logedin;}




	// -------------------------------------------------------------------------------------------------
	std::string getIpAdd();
	std::string getHostname();

	void SetNickname(std::string& nickName);
	void setLogedin(bool value);
	void SetUsername(std::string& username);
	void addToBuffer(std::string recived_data);
	void setRegistered(bool value);
	//---------------//Methods
	void clearBuffer();
	void AddChannelInvite(std::string &chname);
	void RmChannelInvite(std::string &chname);
};

#endif