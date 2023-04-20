# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/10/27 17:07:45 by junhjeon          #+#    #+#              #
#    Updated: 2023/04/19 15:38:23 by junhjeon         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX = c++
CPPFLAGS = #-Wall -Wextra -Werror -std=c++98 -g3
SRC_DIR = .
NAME = webserv
SRCS = ./main.cpp \
	./ServerConfig/ServerConfig.cpp \
	./ServerEngine/ServerEngine.cpp \
	./ServerEngine/ServerEngineProcess.cpp\
	./Request/Request.cpp \
	./Response/Response.cpp	\
	./CGI/CgiHandler.cpp	\
	./KqueueUdata.cpp \
	./ParsingUtility.cpp 


OBJS = $(SRCS:.cpp=.o)
INCLUDE = .

all : $(NAME)
		
.cpp.o:
	$(CXX) -c $(CPPFLAGS) -I $(INCLUDE) -o $@ $< 

$(NAME) : $(OBJS) 
	$(CXX) $(CPPFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) 

fclean: clean
	rm -f $(NAME)

re:
	make fclean;
	make all;

.PHONY: clean all fclean re
