CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = chatroom_client
OBJS = ../log/*.cpp  \
        ../chatroom_client.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)




