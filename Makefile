FLAGS = -Wall -Werror -ansi -pedantic

# if [ -a bin ] ; \
# then \
#      rm bin ; \
# fi;

# mkdir bin
Remove_bin = rm -rf bin

MAKE_bin = mkdir bin

all:
	${Remove_bin}
	${MAKE_bin}

	g++ $(FLAGS) src/rshell.cpp -o bin/rshell

#put executable into bin