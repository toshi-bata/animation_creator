ifeq ($(shell uname),Linux)
	BIN = bin/HttpdServer
else
	BIN = /Users/toshi/HttpdServer
endif

OBJS = main.cpp utilities.cpp
CC = g++ -std=c++11

ifeq ($(shell uname),Linux)
	PATH_TO_INCLUDES = -I /usr/local/include
else
	PATH_TO_INCLUDES = -I /usr/local/include
endif

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