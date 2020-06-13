# 50000001_BOOS的Makefile文件
CPP = g++
CPPFLAGS = -I ../../include -I ../../include/ServerCommon -I ../../include/curl -I ../../include/DataBase -I ../../include/libevent -I ../../include/hiredis -I ../../protobuf -I /usr/local/include -g -Wall -O3 -fpic -std=c++11 

LDFLAGS = -L ../../bin -shared 
LIBS = -lKernel -lCommon -lGameManage -ljson -lprotobuf -ljemalloc

BINDIR := ../../bin/
BIN := 50000001.so
OBJ := GameLogic.o GameServer.o PolygonShape.o TableFrameSink.o

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $^ $(CPPFLAGS)

#%.o:../protobuf/%.cc
#	$(CPP) -c $^ $(CPPFLAGS)

install:
	cp 50000001_s.ini ../../bin/
	cp FishLKPath.pat ../../bin/
	@echo "50000001_BOOS生成在../../bin/目录中"

clean:
	rm -rf *.o
	
