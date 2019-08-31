#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 去除编译错误
#define _CRT_SECURE_NO_WARNINGS

// 头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <WinSock2.h>
#include <windows.h>

//导出定义, 定义输出
#ifdef KERNEL_EXPORTS
#define KERNEL_CLASS __declspec(dllexport)
#else
#define KERNEL_CLASS __declspec(dllimport)
#endif
