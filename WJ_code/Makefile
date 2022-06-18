CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = WebServer
OBJS = ./log/*.cpp  \
        ./server/*.cpp\
        ./main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./$(TARGET)  -pthread -lmysqlclient
	./$(TARGET)
clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
	




