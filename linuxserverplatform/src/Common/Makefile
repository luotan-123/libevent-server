# Common的Makefile文件
CPP = g++
CPPFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 -g -Wall -O3 -fpic -std=c++11 
CFLAGS = -I ../include -I ../include/ServerCommon -I ../include/curl -I ../include/DataBase -I ../include/libevent -I ../include/hiredis -I ../lua -I ../protobuf -I /usr/local/include -DLUA_COMPAT_5_1 -g -Wall -O3 -fpic -std=c11

LDFLAGS = -L ../bin -shared 
LIBS = -lpthread -lKernel -lhiredis -lcurl -lprotobuf -ljemalloc -levent -levent_pthreads -llua -lmysqlclient 

BINDIR := ../bin/
BIN := libCommon.so
OBJ :=  BillManage.o HttpServer.o MD5.o MyCurl.o RedisBase.o RedisCenter.o RedisCommon.o RedisLoader.o RedisLogon.o RedisPHP.o Util.o jemalloc_cpp.o auxiliar.o buffer.o except.o inet.o io.o ls_mysql.o luasocket.o luasql.o mime.o options.o pb.o select.o tcp.o timeout.o udp.o unix.o usocket.o AsyncEventMsg.pb.o

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $^ $(CPPFLAGS)

jemalloc_cpp.o:../jemalloc/jemalloc_cpp.cpp
	$(CPP) -c $^ $(CPPFLAGS)

%.o:../protobuf/%.cc
	$(CPP) -c $^ $(CPPFLAGS)

%.o:../lua/%.c
	$(CPP) -c -x c $^ $(CFLAGS)

install:
	cp $(BINDIR)$(BIN) /usr/local/lib

clean:
	rm -rf *.o
	
