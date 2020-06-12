# 说明
编译环境：win10 + vs2019 + ssh + centos7

vs2019必须远程连接linux

Documentation里面，有linux开发环境安装相关文档

# projects
c++ 项目
基于libevent、jemalloc的棋牌游戏服务器框架
包含4种游戏，扑鱼、斗地主、麻将

# 用到的轮子：
如果之前已经安装过以下第三方库，就没必要安装

用到的轮子：

1、curl （centos下已经编译好库，不需要安装）

2、jemalloc （必须安装）

3、libevent（centos下已经编译好库，不需要安装）

4、libjson （centos下已经编译好库，不需要安装）

5、protobuf（必须安装）

6、redis （选择安装）

hiredis 已经编译成静态库。redis-server、redis-cli、redis-sentinel自己部署。
如果安装，由于redis依赖库hiredis、jemalloc、lua本服务器也用到。lua尽量用最新版安装，不要用redis自带的。hiredis最好安装。jemalloc可以选择redis自带或者最新版。

7、libmysqlclient（centos下已经编译好库，不需要安装）

8、lua  (需要下载安装，包括一些其它杂项，参考lua安装相关文档)
