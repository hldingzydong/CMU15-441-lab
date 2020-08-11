CC=gcc
CFLAGS=-I.
DEPS = parse.h y.tab.h
OBJ = y.tab.o lex.yy.o parse.o example.o
FLAGS = -g -Wall

default:all

all: example echo_server echo_client
# $^指所有前置条件
lex.yy.c: lexer.l
	flex $^

y.tab.c: parser.y
	yacc -d $^

# $@指当前target
# $<指第一个前置条件,比如规则为t: p1 p2, 那么$<指p1
%.o: %.c $(DEPS)
	$(CC) $(FLAGS) -c -o $@ $< $(CFLAGS)

example: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

echo_server:
	$(CC) echo_server.c parse.c y.tab.c lex.yy.c -g -o echo_server ##-Wall -Werror

echo_client:
	$(CC) echo_client.c -g -o echo_client -Wall -Werror

clean:
	rm -f *~ *.o example lex.yy.c y.tab.c y.tab.h echo_server echo_client
