# 23510004_SXTDH的Makefile文件
CPP = g++
CPPFLAGS = -I ../../include -I ../../include/ServerCommon -I ../../include/curl -I ../../include/DataBase -I ../../include/libevent -I ../../include/hiredis -I ../../protobuf -I /usr/local/include -g -Wall -O3 -fpic -std=c++11 

LDFLAGS = -L ../../bin -shared 
LIBS = -lKernel -lCommon -lGameManage -ljson -lprotobuf -ljemalloc

BINDIR := ../../bin/
BIN := 23510004.so
OBJ := CheckCPGAction.o CheckCPGActionEx.o CheckHuPai.o CheckHuPaiEx.o GameData.o GameDataEx.o ServerManage.o SHSrvDLL.o

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $^ $(CPPFLAGS)

#%.o:../protobuf/%.cc
#	$(CPP) -c $^ $(CPPFLAGS)

install:
	cp 23510004_s.ini ../../bin/
	@echo "23510004_SXTDH生成在../../bin/目录中"

clean:
	rm -rf *.o
	
