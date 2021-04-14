# Common的Makefile文件

# 编译版本，可选debug和release
#build_ver=debug

ifeq ($(build_ver),release)
BUILD_OPT= -g1 -O2 
else
BUILD_OPT= -g2 -O0
endif


CPP = g++
CPPFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 $(BUILD_OPT) -Wall -fpic -std=c++11 
CFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 $(BUILD_OPT) -Wall -fpic -std=c11

# 设置默认加载库位置
CURDIR := $(shell pwd)
PARENT_CURDIR = $(shell dirname $(CURDIR))
PARENT_BIN := $(PARENT_CURDIR)/bin/

LDFLAGS = -L ../bin -shared -Wl,-rpath=$(PARENT_BIN)
LIBS = -lpthread -lKernel -lhiredis -lcurl -lprotobuf -ljemalloc -levent -levent_pthreads -llua -lmysqlclient 

BINDIR := ../bin/
BIN := libCommon.so
OBJ :=  BillManage.o HttpServer.o MD5.o MyCurl.o RedisBase.o RedisCenter.o RedisCommon.o RedisLoader.o RedisLogon.o RedisPHP.o Util.o jemalloc_cpp.o auxiliar.o buffer.o except.o inet.o io.o ls_mysql.o luasocket.o luasql.o mime.o options.o pb.o select.o tcp.o timeout.o udp.o unix.o usocket.o AsyncEventMsg.pb.o
DEP = $(OBJ:.o=.dep)

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $< $(CPPFLAGS)

jemalloc_cpp.o:../jemalloc/jemalloc_cpp.cpp
	$(CPP) -c $< $(CPPFLAGS)

%.o:../protobuf/%.cc
	$(CPP) -c $< $(CPPFLAGS)

%.o:../lua/%.c
	$(CPP) -c -x c $< $(CFLAGS)


# 生成依赖项
%.dep:%.cpp
	$(CPP) -MM $< $(CPPFLAGS) > $@

%.dep:../protobuf/%.cc
	$(CPP) -MM $< $(CPPFLAGS) > $@

%.dep:../lua/%.c
	$(CPP) -MM -x c $< $(CFLAGS) > $@
	
-include $(DEP)


install:
	cp $(BINDIR)$(BIN) /usr/local/lib

clean:
	rm -rf *.o *.dep
	
