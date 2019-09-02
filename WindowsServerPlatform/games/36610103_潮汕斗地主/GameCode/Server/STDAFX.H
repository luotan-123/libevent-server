// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <WinSock2.h>
#include <windows.h>

#define KERNEL_CLASS __declspec(dllimport)

#pragma warning(disable:4251)

#pragma comment(lib, "Kernel.lib")
#pragma comment(lib, "Common.lib") 
#pragma comment(lib, "GameManage.lib")

#include "../GameMessage/UpgradeMessage.h"
#include "GameDesk.h"
#include "CommonUse.h"