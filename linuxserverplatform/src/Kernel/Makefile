# Kernel的Makefile文件

# 编译版本，可选debug和release
# build_ver=debug

ifeq ($(build_ver),release)
BUILD_OPT= -g1 -O3 
else
BUILD_OPT= -g2 -O0
endif


CPP = g++
CPPFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I /usr/local/include $(BUILD_OPT) -Wall -fpic -std=c++11 

LDFLAGS = -L ../bin -shared 
LIBS = -lpthread -lcurl -lmysqlclient -levent -levent_pthreads -ljemalloc 

BINDIR := ../bin/
BIN := libKernel.so
OBJ := jemalloc_cpp.o base64.o configManage.o DataBase.o DataLine.o Exception.o Function.o GameLogManage.o GServerConnect.o INIFile.o Lock.o log.o MysqlHelper.o ServerTimer.o sha1.o TcpConnect.o TCPSocket.o WebSocket.o Xor.o shmem.o CXObj.o UnlockQueue.o RRlockQueue.o

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
	
