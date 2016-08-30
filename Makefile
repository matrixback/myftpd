.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=myftpd
OBJS=main.o session.o tool.o nobody.o ftpcmd.o unsock.o
LIBS=-lcrypt

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)

