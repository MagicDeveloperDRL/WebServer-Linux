CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = chatroom_server
OBJS = ../log/*.cpp  \
        ../chatroom_server.cpp
all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)




