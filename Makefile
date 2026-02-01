ifeq (${HOSTTYPE},)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

#-------------- SOURCES --------------#

SRC =		malloc.c \
			free.c \
			utils.c \
			print_mem.c \
			scan_memory.c \

SRC_DIR =	src

OBJ	=	${SRC:%.c=${OBJ_DIR}/%.o}

OBJ_DIR	=	obj

#------------ COMPILATION ------------#

NAME =		libft_malloc.so

LIBHOST =	libft_malloc_${HOSTTYPE}.so

CC =		gcc

FLAGS =		-Wall -Wextra -Werror -g

INCLUDES =	${SRC_DIR}/malloc.h \
			${SRC_DIR}/utils.h \
			ft_malloc.h \

LIBFT_NAME =	libft.a

LIBFT_DIR =		libft

LIBFT_INCLUDE =	${LIBFT_DIR}/libft.h

LIBFT =		${LIBFT_DIR}/${LIBFT_NAME}

#--------------- RULES --------------#

all:		${NAME}

${OBJ_DIR}/%.o:	${SRC_DIR}/%.c ${INCLUDES} | ${OBJ_DIR}
				${CC} ${FLAGS} -fPIC -c $< -o $@ -I.

${NAME}:	${OBJ} ${INCLUDES}
			${CC} ${FLAGS} ${OBJ} -shared -o ${LIBHOST}
			ln -sf ${LIBHOST} ${NAME}

${OBJ_DIR}:	
			@mkdir -p ${OBJ_DIR}

clean:		
			rm -rf ${OBJ_DIR}

fclean:		clean
			rm -f ${NAME} ${LIBHOST}

re:		fclean all

.PHONY: clean fclean all re	
