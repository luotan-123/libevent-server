#pragma once
#include   <stdarg.h>

//日志类
class GameLog
{
public:
	static void OutputFile(const char *strOutputString, ...);	 //输出日志文件
	static void OutputString(const char * strOutputString, ...); //将字符串输出到控制台
	static void SFile(const char * strOutputString, ...);		 //将字符串输出到文件，此函数运营版也可以输出
private:
};
