#pragma once

// 公共头文件
#include <jemalloc/jemalloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <limits.h>

// 常用类型别名
typedef unsigned int UINT;
typedef unsigned long int ULONG;
typedef unsigned char BYTE;
typedef unsigned long HANDLE;
typedef bool BOOL;
typedef long long __int64;

// 版本定义
#define VER_MAIN 3
#define VER_MIDDLE 1
#define VER_RESVERSE 1
#define VER_BUILDTIME "2020"

// 基础常量定义
#define TRUE true
#define FALSE false

// 条件编译，生成jemalloc内存泄露分析文件
#define JEMALLOC_PROFILE_MEMORY