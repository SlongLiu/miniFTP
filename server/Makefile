.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=server
OBJS=server.o session_ftp.o execute_func.o handle_ftp.o priv_sock.o strtools.o  headers.o pasv_ftp.o data_trans.o
LIB=-lcrypt
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
