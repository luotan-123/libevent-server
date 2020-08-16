# GameManage的Makefile文件
CPP = g++
CPPFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 -g -Wall -O3 -fpic -std=c++11 
CFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 -g -Wall -O3 -fpic -std=c11

# 设置默认加载库位置
CURDIR := $(shell pwd)
PARENT_CURDIR = $(shell dirname $(CURDIR))
PARENT_BIN := $(PARENT_CURDIR)/bin/

LDFLAGS = -L ../bin -shared -Wl,-rpath=$(PARENT_BIN)
LIBS = -lpthread -lKernel -lCommon -ljson -lprotobuf -ljemalloc -lhiredis

BINDIR := ../bin/
BIN := libGameManage.so
OBJ := BaseMainManage.o GameDataBaseHandle.o GameDesk.o GameMainManage.o gameUserManage.o jemalloc_cpp.o

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $^ $(CPPFLAGS)

jemalloc_cpp.o:../jemalloc/jemalloc_cpp.cpp
	$(CPP) -c $^ $(CPPFLAGS)

#%.o:../protobuf/%.cc
#	$(CPP) -c $^ $(CPPFLAGS)

#%.o:../lua/%.c
#	$(CPP) -c -x c $^ $(CFLAGS)

install:
	cp $(BINDIR)$(BIN) /usr/local/lib

clean:
	rm -rf *.o
	
