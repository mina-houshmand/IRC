#ifndef BONUS_BOT_HPP
#define BONUS_BOT_HPP

#include <string>
#include <vector>
#include <map>

class Server;

class TriviaBot {
public:
	struct TriviaQuestion {
		std::string question;
		std::string choices[4];
		int rightAnswer; // 1..4
	};

	struct TriviaSession {
		int fd;
                bool waitingForCount;
                bool inProgress;
                int currentQuestion;
                int totalQuestions;
                int score;
                std::vector<int> questionOrder;
        };

private:
	bool enabled;
	std::string botNick;
	std::vector<TriviaQuestion> triviaQuestions;
	std::map<int, TriviaSession> triviaSessions;

	static std::string IntToString(int value);

public:
	TriviaBot();
	bool IsEnabled() const;
	void Enable(Server &server);
	bool LoadQuestionsFromFile(const std::string &path);
	void InitializeQuestions();
	void SendBotChannelMessage(Server &server, const std::string &channelName, const std::string &message, int excludeFd = -1);
	void SendBotPrivateMessage(Server &server, int fd, const std::string &message);
	void SendTriviaQuestion(Server &server, int fd);
	void EndTriviaSession(int fd);
	bool ProcessMessage(Server &server, int fd, const std::string &target, const std::string &message);
};

#endif
