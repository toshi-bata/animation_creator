BIN = bin/HttpdServer

OBJS = main.cpp
CC = g++ -std=c++11

PATH_TO_INCLUDES = -I /usr/local/include

PATH_TO_LIBMHD_LIBS = /usr/local/lib

LDLIBS = -ldl -lstdc++ \
	 -L$(PATH_TO_LIBMHD_LIBS) \
	 -lmicrohttpd \
	 -lstdc++fs

$(BIN): $(OBJS)
	$(CC) $(PATH_TO_INCLUDES) $(OBJS) -o $(BIN) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(BIN)