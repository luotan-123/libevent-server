#pragma once

// Í·ÎÄ¼þ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <process.h>
#include <time.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <array>
#include <math.h>
#include <new.h>
#include <WinSock2.h>
#include <windows.h>
#include <DbgHelp.h> 

#define VER_MAIN 1
#define VER_MIDDLE 0
#define VER_RESVERSE 0
#define VER_BUILDTIME "2018"

#pragma warning(disable:4251)

#pragma comment(lib, "Kernel.lib")
#pragma comment(lib, "Common.lib") 

#define KERNEL_CLASS __declspec(dllimport)