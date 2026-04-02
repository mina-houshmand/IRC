#include "../INC/Server.hpp"
#include "bot.hpp"
#include <algorithm>
#include <ctime>
#include <sstream>
#include <fstream>

TriviaBot::TriviaBot()
	: enabled(false), botNick("trivia_bot")
{
}

bool TriviaBot::IsEnabled() const
{
	return enabled;
}

std::string TriviaBot::IntToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

void TriviaBot::Enable(Server &server)
{
	if (enabled)
		return;

	enabled = true;
	if (!LoadQuestionsFromFile("BONUS/questions.txt"))
	{
		InitializeQuestions();
	}

	Channel triviaChannel;
	triviaChannel.SetName("#trivia");
	triviaChannel.SetTopicName("Trivia bot channel");

	Client botClient(botNick, "bot", -1);
	triviaChannel.add_admin(botClient);
	server.AddChannel(triviaChannel);

	std::cout << "\033[0;35m[TRIVIA BOT]\033[0m channel #trivia created and bot registered as admin" << std::endl;
}

bool TriviaBot::LoadQuestionsFromFile(const std::string &path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return false;

	triviaQuestions.clear();
	std::string line;

	while (std::getline(file, line))
	{
		if (line.empty() || line[0] == '#')
			continue;

		std::vector<std::string> parts;
		std::string chunk;
		std::istringstream lineStream(line);
		while (std::getline(lineStream, chunk, '|'))
			parts.push_back(chunk);

		if ((int)parts.size() != 6)
			continue;

		TriviaQuestion q;
		q.question = parts[0];
		for (int i = 0; i < 4; ++i)
			q.choices[i] = parts[i + 1];

		q.rightAnswer = std::atoi(parts[5].c_str());
		if (q.rightAnswer < 1 || q.rightAnswer > 4)
			continue;

		triviaQuestions.push_back(q);
		if ((int)triviaQuestions.size() >= 100)
			break;
	}

	return !triviaQuestions.empty();
}

void TriviaBot::InitializeQuestions()
{
	triviaQuestions.clear();
	for (int i = 1; i <= 100; ++i)
	{
		TriviaQuestion q;
		q.question = "What is " + IntToString(i) + " + " + IntToString(i) + "?";
		int correct = i + i;
		q.choices[0] = IntToString(correct);
		q.choices[1] = IntToString(correct + 1);
		q.choices[2] = IntToString(correct - 1);
		q.choices[3] = IntToString(correct + 2);
		q.rightAnswer = 1;
		triviaQuestions.push_back(q);
	}
}

void TriviaBot::SendBotChannelMessage(Server &server, const std::string &channelName, const std::string &message, int excludeFd)
{
	Channel *channel = server.GetChannel(channelName);
	if (!channel)
		return;

	std::string rpl = ":" + botNick + "!~bot@localhost PRIVMSG " + channelName + " :" + message + "\r\n";
	channel->sendTo_all(rpl, excludeFd);
}

void TriviaBot::SendBotPrivateMessage(Server &server, int fd, const std::string &message)
{
	Client *client = server.GetClient(fd);
	if (!client)
		return;

	std::string rpl = ":" + botNick + "!~bot@localhost PRIVMSG " + client->GetNickName() + " :" + message + "\r\n";
	server._sendResponse(rpl, fd);
}

void TriviaBot::SendTriviaQuestion(Server &server, int fd)
{
	std::map<int, TriviaSession>::iterator it = triviaSessions.find(fd);
	if (it == triviaSessions.end())
		return;

	TriviaSession &session = it->second;
	if (!session.inProgress)
		return;

	int qIndex = session.questionOrder[session.currentQuestion];
	if (qIndex < 0 || qIndex >= (int)triviaQuestions.size())
		return;

	TriviaQuestion &q = triviaQuestions[qIndex];
	int questionNumber = session.currentQuestion + 1;

	SendBotChannelMessage(server, "#trivia", "Question " + IntToString(questionNumber) + " of 10: " + q.question);
	SendBotChannelMessage(server, "#trivia", "1) " + q.choices[0]);
	SendBotChannelMessage(server, "#trivia", "2) " + q.choices[1]);
	SendBotChannelMessage(server, "#trivia", "3) " + q.choices[2]);
	SendBotChannelMessage(server, "#trivia", "4) " + q.choices[3]);
}

void TriviaBot::EndTriviaSession(int fd)
{
	std::map<int, TriviaSession>::iterator it = triviaSessions.find(fd);
	if (it != triviaSessions.end())
		triviaSessions.erase(it);
}

bool TriviaBot::ProcessMessage(Server &server, int fd, const std::string &target, const std::string &message)
{
	if (!enabled || target != "#trivia")
		return false;

	Client *client = server.GetClient(fd);
	if (!client)
		return true;

	std::string text = message;
	if (!text.empty() && text[0] == ':')
		text = text.substr(1);

	for (size_t i = 0; i < text.size(); ++i)
		text[i] = std::tolower(text[i]);

	bool hasSession = (triviaSessions.find(fd) != triviaSessions.end() && triviaSessions[fd].inProgress);

	if (!hasSession)
	{
		if (text != "start")
		{
			SendBotChannelMessage(server, "#trivia", "To start the trivia write start", fd);
			return true;
		}

		TriviaSession session;
		session.fd = fd;
		session.inProgress = true;
		session.currentQuestion = 0;
		session.score = 0;

		session.questionOrder.clear();
		for (int i = 0; i < (int)triviaQuestions.size(); i++)
			session.questionOrder.push_back(i);

		for (int i = (int)session.questionOrder.size() - 1; i > 0; i--) {
			int j = std::rand() % (i + 1);
			std::swap(session.questionOrder[i], session.questionOrder[j]);
		}

		if (session.questionOrder.size() > 10)
			session.questionOrder.resize(10);

		triviaSessions[fd] = session;
		SendBotChannelMessage(server, "#trivia", "Trivia started! Answer by typing 1-4.");
		SendTriviaQuestion(server, fd);
		return true;
	}

	if (text != "1" && text != "2" && text != "3" && text != "4")
	{
		SendBotChannelMessage(server, "#trivia", "Invalid answer. Please write 1, 2, 3 or 4.", fd);
		return true;
	}

	int selected = std::atoi(text.c_str());
	TriviaSession &session = triviaSessions[fd];
	int qIndex = session.questionOrder[session.currentQuestion];
	TriviaQuestion &q = triviaQuestions[qIndex];

	if (selected == q.rightAnswer)
	{
		session.score++;
		SendBotChannelMessage(server, "#trivia", "Right answer!");
	}
	else
	{
		SendBotChannelMessage(server, "#trivia", "Wrong answer. Correct is " + IntToString(q.rightAnswer) + ".");
	}

	session.currentQuestion++;
	if (session.currentQuestion >= 10)
	{
		SendBotChannelMessage(server, "#trivia", "Trivia over! Your score: " + IntToString(session.score) + " / 10");
		SendBotChannelMessage(server, "#trivia", "To start again write start");
		EndTriviaSession(fd);
		return true;
	}

	SendTriviaQuestion(server, fd);
	return true;
}
