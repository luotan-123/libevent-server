# GameManage的Makefile文件

# 编译版本，可选debug和release
#build_ver=debug

ifeq ($(build_ver),release)
BUILD_OPT= -g1 -O3 
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
LIBS = -lpthread -lKernel -lCommon -ljson -lprotobuf -ljemalloc -lhiredis

BINDIR := ../bin/
BIN := libGameManage.so
OBJ := BaseMainManage.o GameDataBaseHandle.o GameDesk.o GameMainManage.o gameUserManage.o jemalloc_cpp.o
DEP = $(OBJ:.o=.dep)

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $< $(CPPFLAGS)

jemalloc_cpp.o:../jemalloc/jemalloc_cpp.cpp
	$(CPP) -c $< $(CPPFLAGS)

#%.o:../protobuf/%.cc
#	$(CPP) -c $< $(CPPFLAGS)

#%.o:../lua/%.c
#	$(CPP) -c -x c $< $(CFLAGS)


# 生成依赖项
%.dep:%.cpp
	$(CPP) -MM $< $(CPPFLAGS) > $@

#%.dep:../protobuf/%.cc
#	$(CPP) -MM $< $(CPPFLAGS) > $@

#%.dep:../lua/%.c
#	$(CPP) -MM -x c $< $(CFLAGS) > $@
	
-include $(DEP)


install:
	cp $(BINDIR)$(BIN) /usr/local/lib

clean:
	rm -rf *.o *.dep
	
