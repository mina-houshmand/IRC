#include "../INC/Server.hpp"

/*reads each line from the input stream (stm) into the variable line

example:	 "NICK John\r\nUSER johnd 0 * :John Doe\r\nJOIN #channel1\r\n"
store it like this:
		vec = {"NICK John", "USER johnd 0 * :John Doe", "JOIN #channel1"};
*/

std::vector<std::string> Server::split_recivedcmd(std::string str)
{
    std::vector<std::string> vec;
    
    if (str.empty())
        return vec;
    
    std::istringstream stm(str);
    std::string line;

    while(std::getline(stm, line))
    {
        size_t pos = line.find_first_of("\r\n");
        if(pos != std::string::npos)
            line = line.substr(0, pos);
        
        if (!line.empty())
            vec.push_back(line);
    }
    return vec;
}

/*     stm >> token
This code splits the input string (cmd) into individual tokens (words) based on whitespace and stores them in a vector (vec).
The >> operator reads from the stream until it encounters a whitespace character (space, tab, newline, etc.).
After extracting the token, the stream's internal pointer moves to the next word.
The loop continues until the end of the stream is reached.
{"heloo", "this"}
*/

std::vector<std::string> Server::split_cmd(std::string& cmd)
{
	std::vector<std::string> cmds_container;
	std::istringstream stream(cmd);
	std::string tokenized_cmd_with_spaces;

	while(stream >> tokenized_cmd_with_spaces)
	{
		cmds_container.push_back(tokenized_cmd_with_spaces);
		tokenized_cmd_with_spaces.clear();
	}
	return cmds_container;
}
