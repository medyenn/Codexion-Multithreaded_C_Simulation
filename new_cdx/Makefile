NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -I src

SRCS		= src/0_main.c \
			  src/1_parser.c \
			  src/2_initializer.c \
			  src/3_simulator.c \
			  src/4_coder.c \
			  src/5_dongle.c \
			  src/6_heap.c \
			  src/7_helper.c \
			  src/8_monitor.c \
			  src/9_cleaner.c \
			  src/utils.c \
			  src/utils2.c \

OBJS		= $(SRCS:.c=.o)


all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c src/codexion.h
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re