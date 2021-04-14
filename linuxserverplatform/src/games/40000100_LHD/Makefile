# 40000100_LHD的Makefile文件

# 编译版本，可选debug和release
# build_ver=debug

ifeq ($(build_ver),release)
BUILD_OPT= -g1 -O3 
else
BUILD_OPT= -g2 -O0
endif


CPP = g++
CPPFLAGS = -I ../../include -I ../../include/ServerCommon -I ../../include/curl -I ../../include/DataBase -I ../../include/libevent -I ../../include/hiredis -I ../../protobuf -I /usr/local/include $(BUILD_OPT) -Wall -fpic -std=c++11 

LDFLAGS = -L ../../bin -shared 
LIBS = -lKernel -lCommon -lGameManage -ljson -lprotobuf -ljemalloc

BINDIR := ../../bin/
BIN := 40000100.so
OBJ := GameServer.o NewServerManage.o UpGradeLogic.o
DEP = $(OBJ:.o=.dep) 

$(BINDIR)$(BIN):$(OBJ)
	$(CPP) -o $@  $(LDFLAGS) $^ $(LIBS)

%.o:%.cpp
	$(CPP) -c $< $(CPPFLAGS)

#%.o:../protobuf/%.cc
#	$(CPP) -c $< $(CPPFLAGS)

install:
	cp 40000100_s.ini ../../bin/
	@echo "40000100_LHD生成在../../bin/目录中"


# 生成依赖项
%.dep:%.cpp
	$(CPP) -MM $< $(CPPFLAGS) > $@

-include $(DEP)


clean:
	rm -rf *.o *.dep
	
