#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include "Client.hpp"
#include "Channel.hpp"
#include "replies.hpp"
#include "../BONUS/bot.hpp"


#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
#define BLU "\e[1;34m"

class Client;
class Channel;

class Server
{
private:
	int port;
	int server_socket_fd;
	static bool Signal;
	std::string password;
	std::vector<Client> clients;
	std::vector<Channel> channels;
	std::vector<struct pollfd> fds;
	struct sockaddr_in add;
	struct sockaddr_in cliadd;
	struct pollfd new_cli;

	// Trivia bot helper object
	TriviaBot triviaBot;

public:
	Server();
	~Server();
	Server(Server const &src);
	Server &operator=(Server const &src);

	int GetPort(){return this->port;}
	int GetFd(){return this->server_socket_fd;}
	void disconnectClient(int fd){
		printStatus(RED, "Client", fd, "Disconnected");
		shutdown(fd, SHUT_RDWR);
		RmChannels(fd);
		RemoveClient(fd);
		RemoveFds(fd);
		close(fd);
	}
	//parsing
	std::vector<std::string> split_recivedcmd(std::string str);
	void handleClientCommand(std::string &cmd, int fd);
	std::vector<std::string> split_cmd( std::string &str);
	void trimLeadingWhitespace(std::string &cmd);
	void cmd_toUpper(std::string &command);


	//printing stuff
	void printStatus(const std::string& color, const std::string& type, int fd, const std::string& status){
		std::cout << color << type << " <" << fd << "> " << status << WHI << std::endl;
	}
	void printMessage(const std::string& message){
		std::cout << BLU << message << std::endl;
	}
	void printError(const std::string& message){
		std::cerr << RED << "[ERROR] " << message << WHI << std::endl;
	}



	// ---------------------------------------------------------------------------------------
	//---------------//Getters
	static bool isBotfull;
	std::string GetPassword();
	Client *GetClient(int current_fd);
	Client *GetClientNick(std::string nickname);
	Channel *GetChannel(std::string name);
	//---------------//Setters
	void SetFd(int server_socket_fd);
	void SetPort(int port);
	void SetPassword(std::string password);
	void AddClient(Client newClient);
	void AddChannel(Channel newChannel);
	void AddFds(pollfd newFd);
	void set_username(std::string& username, int fd);
	void set_nickname(std::string cmd, int fd);
	//---------------//Remove Methods
	void RemoveClient(int fd);
	void RemoveChannel(std::string name);
	void RemoveFds(int fd);
	void RmChannels(int fd);
	//---------------//Send Methods
	void senderror(int code, std::string clientname, int fd, std::string msg);
	void senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg);
	void _sendResponse(std::string response, int fd);
	//---------------//Close and Signal Methods
	static void SignalHandler(int signum);
	void close_fds();


	//---------------//Server Methods
	void server_config(std::string port, std::string pass);
	bool isSocketReadable(const pollfd& pfd);

	void set_sever_socket();
	void initializeAddress(struct sockaddr_in &address, int port);
	void addSocketToPoll(int fd);

	int performPoll_request();

	//new connection request
	void new_connection_request();
	int	 acceptNewClient();
	bool setupClientSocket(int fd);
	void addClientToPoll(int fd);
	void addClientToServer(const Client &client);
	void initializeClient(Client &client, int fd);



	//data transform
	void data_transform(int fd);
	void processClientCommands(const std::string &buffer, int fd);



	//---------------//Authentification Methods
	// bool BypassForBot(std::string cmd, int fd);
	// bool notregistered(int fd);
	bool isClientRegistered(int fd);
	bool nickNameInUse(std::string& nickname);
	bool is_validNickname(std::string& nickname);
	void client_authen(int fd, std::string pass);
	//---------------------------//JOIN CMD
	void	JOIN(std::string cmd, int fd);
	bool 	TokenizeJoinCmd(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd);
	void 	ProcessJoinChannel(const std::pair<std::string, std::string> &channelKeyPair, int fd);
	void 	CreateNewChannel(const std::pair<std::string, std::string> &channelKeyPair, int fd);
	void 	HandleExistingChannel(const std::pair<std::string, std::string> &channelKeyPair, size_t channelIndex, int fd);
	int 	numberOfChannelsThatJoined(const std::string &nickName);
	void 	NotifyJoin(Client *client, Channel &channel);

	//---------------------------//HELP CMD
	void	HELP(int fd);
	//---------------------------//PART CMD
	void	PART(std::string cmd, int fd);
	bool	SplitCmdPart(std::string cmd, std::vector<std::string> &tmp, std::string &reason, int fd);
	void	ProcessPartChannel(const std::string &channelName, int fd, const std::string &reason);

	//---------------------------//KICK CMD
	void	KICK(std::string cmd, int fd);
	//---------------------------//PRIVMSG CMD
	void	PRIVMSG(std::string cmd, int fd);
	bool	SplitPrivMsg(std::string cmd, std::vector<std::string> &targets, std::string &message, int fd);
	void	SendToChannel(const std::string &channelName, int fd, const std::string &message);
	void	SendToUser(const std::string &nickname, int fd, const std::string &message);
	//---------------------------//QUIT CMD
	void	QUIT(std::string cmd, int fd);
	std::string	ParseQuitReason(std::string cmd);
	void	BroadcastQuit(int fd, const std::string &reason);
	//---------------------------//MODE CMD
	void 		mode_command(std::string& cmd, int fd);
	std::string invite_only(Channel *channel, char opera, std::string chain);
	std::string topic_restriction(Channel *channel ,char opera, std::string chain);
	std::string password_mode(std::vector<std::string> splited, Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string& arguments);
	std::string operator_privilege(std::vector<std::string> splited, Channel *channel, size_t& pos, int fd, char opera, std::string chain, std::string& arguments);
	std::string channel_limit(std::vector<std::string> splited, Channel *channel, size_t &pos, char opera, int fd, std::string chain, std::string& arguments);
	bool		isvalid_limit(std::string& limit);
	std::string mode_toAppend(std::string chain, char opera, char mode);
	std::vector<std::string> splitParams(std::string params);
	void getCmdArgs(std::string cmd,std::string& name, std::string& modeset ,std::string &params);
	//---------------------------//TOPIC CMD
	void	Topic(std::string &cmd, int &fd);
	std::string	ParseTopic(std::string &cmd);
	void	Invite(std::string &cmd, int &fd);

	// Trivia bot methods
	void	EnableTriviaBot();
	bool	ProcessTriviaMessage(int fd, const std::string &target, const std::string &message);
	void	SendTriviaQuestion(int fd);
	void	EndTriviaSession(int fd);
};

#endif