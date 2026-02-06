NAME_BOUNUS = bot
NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g
DEPFLAGS = -MMD -MP

SRCS = main.cpp SRC/Authenti.cpp SRC/Channel.cpp SRC/Client.cpp SRC/Server.cpp \
	CMD/INVITE.cpp CMD/JOIN.cpp CMD/HELP.cpp CMD/KICK.cpp CMD/MODE.cpp \
	CMD/PART.cpp CMD/PRIVMSG.cpp CMD/QUIT.cpp CMD/TOPIC.cpp 

SRCS_BONUS = BONUS/main.cpp BONUS/player.cpp BONUS/bot.cpp

OBJSDIR = objs

OBJS = $(addprefix $(OBJSDIR)/,$(SRCS:.cpp=.o))
OBJS_BONUS = $(addprefix $(OBJSDIR)/,$(SRCS_BONUS:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

bonus: $(OBJS_BONUS)
	@$(CC) $(CFLAGS) -o $(NAME_BOUNUS) $(OBJS_BONUS)


$(OBJSDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@


-include $(OBJS:.o=.d) $(OBJS_BONUS:.o=.d)


clean:
	@rm -rf $(OBJSDIR)

fclean: clean
	@rm -f $(NAME) $(NAME_BOUNUS) a.out

re: fclean all

.PHONY: all clean fclean re