CC		= c++

CPPFLAGS	= -Wall -Wextra -Werror -std=c++98

NAME 	= ircserv

SRCS 	= 	bot/BOT.cpp \
			cmds/INVITE.cpp cmds/JOIN.cpp cmds/KICK.cpp cmds/MODE.cpp cmds/NICK.cpp cmds/PASS.cpp cmds/PRIVMSG.cpp cmds/QUIT.cpp cmds/TOPIC.cpp cmds/USER.cpp \
			src/Channel.cpp src/client.cpp src/main.cpp src/Server.cpp \

OBJS	= $(SRCS:%.cpp=$(OBJ_DIR)%.o)

OBJ_DIR = .obj/


all		: $(NAME)

run		:    all
	@if [ -f $(NAME) ]; \
	then clear && /bin/bash -c './$(NAME) "$$@"' __ "8080" "okokok"; \
	fi

$(NAME) : $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -c $< -o $@ -MMD

clean	:
	rm -rf $(OBJ_DIR)

fclean	: clean
	rm -f $(NAME)

re		: fclean all

.PHONY	: clean

-include $(SRCS:%.cpp=$(OBJ_DIR)%.d)
