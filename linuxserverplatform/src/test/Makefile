# test的Makefile文件
CPP = g++
CPPFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 -g -Wall -O3 -std=c++11 
CFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 -g -Wall -O3 -std=c11

# 设置默认加载库位置
CURDIR := $(shell pwd)
PARENT_CURDIR = $(shell dirname $(CURDIR))
PARENT_BIN := $(PARENT_CURDIR)/bin/

LDFLAGS = -L ../bin -Wl,-rpath=$(PARENT_BIN)
LIBS = -lKernel -lCommon -lprotobuf -ljson -lpthread -levent -ljemalloc

BINDIR := ../bin/
BIN := test
OBJ := jemalloc_cpp.o test.pb.o main.o Shm.o TestUnLockQueue.o

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $^ $(CPPFLAGS)

jemalloc_cpp.o:../jemalloc/jemalloc_cpp.cpp
	$(CPP) -c $^ $(CPPFLAGS)

%.o:../protobuf/%.cc
	$(CPP) -c $^ $(CPPFLAGS)

#%.o:../lua/%.c
#	$(CPP) -c -x c $^ $(CFLAGS)

install:
	@echo "test生成在../bin/目录中"

clean:
	rm -rf *.o
	
