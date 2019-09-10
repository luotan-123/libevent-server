#pragma once

// Í·ÎÄ¼þ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <process.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <array>
#include <math.h>
#include <new.h>
#include <WinSock2.h>
#include <windows.h>
#include <DbgHelp.h> 

#define VER_MAIN 2
#define VER_MIDDLE 0
#define VER_RESVERSE 0
#define VER_BUILDTIME "2019"

#pragma warning(disable:4251)

#pragma comment(lib, "Kernel.lib")
#pragma comment(lib, "Common.lib") 
#pragma comment(lib, "GameManage.lib")

#define KERNEL_CLASS __declspec(dllimport)