# Kernel的Makefile文件
CPP = g++
CPPFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I /usr/local/include -g -Wall -O3 -fpic -std=c++11 

LDFLAGS = -L ../bin -shared 
LIBS = -lpthread -lcurl -lmysqlclient -levent -levent_pthreads -ljemalloc 

BINDIR := ../bin/
BIN := libKernel.so
OBJ := jemalloc_cpp.o base64.o configManage.o DataBase.o DataLine.o Exception.o Function.o GameLogManage.o GServerConnect.o INIFile.o Lock.o log.o MysqlHelper.o ServerTimer.o sha1.o TcpConnect.o TCPSocket.o WebSocket.o Xor.o shmem.o

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $^ $(CPPFLAGS)

jemalloc_cpp.o:../jemalloc/jemalloc_cpp.cpp
	$(CPP) -c $^ $(CPPFLAGS)

install:
	cp $(BINDIR)$(BIN) /usr/local/lib

clean:
	rm -rf *.o
	
