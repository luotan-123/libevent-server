#include "main.h"
#include "GameLogonManage.h"
#include "GameLogonModule.h"
#include "Exception.h"
#include "configManage.h"
#include "GameLogManage.h"
#include "commonuse.h"
#include "Util.h"

#pragma comment(lib, "dbghelp.lib")

#define APP_TITLE "GateServer"
#define APP_LOGON_TITLE	"logonServer"

// 登陆服务器（网关服务器）模块
CGameLogonModule	g_LogonServerModule;

inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{
	if (pModuleName == 0)
	{
		return FALSE;
	}

	WCHAR szFileName[_MAX_FNAME] = L"";
	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

	if (_wcsicmp(szFileName, L"ntdll") == 0)
		return TRUE;

	return FALSE;
}

inline BOOL CALLBACK MiniDumpCallback(PVOID  pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT  pOutput)
{
	if (pInput == 0 || pOutput == 0)
		return FALSE;

	switch (pInput->CallbackType)
	{
	case ModuleCallback:
		if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
			if (!IsDataSectionNeeded(pInput->Module.FullPath))
				pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
	case IncludeModuleCallback:
	case IncludeThreadCallback:
	case ThreadCallback:
	case ThreadExCallback:
		return TRUE;
	default:;
	}

	return FALSE;
}

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)
{
	HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;
		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pep;
		mdei.ClientPointers = NULL;

		MINIDUMP_CALLBACK_INFORMATION mci;
		mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
		mci.CallbackParam = 0;

		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);

		CloseHandle(hFile);
	}
}

LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	char szPath[MAX_PATH] = "";
	GetCurrentDirectory(MAX_PATH, szPath);
	sprintf(szPath + strlen(szPath), "%s", "\\Dump\\");
	CreateDirectory(szPath, NULL);

	char szFile[MAX_PATH] = "";
	SYSTEMTIME tm;
	GetLocalTime(&tm);

	sprintf(szFile, "%sError%d-%d-%d&%d-%d-%d.dmp", szPath, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	CreateMiniDump(pExceptionInfo, szFile);

	CWin32Exception::OutputWin32Exception("#### 非常严重的错误，如缓冲区溢出，请在path\\dump目录下查看dump文件。 ####");
	exit(0);

	return EXCEPTION_EXECUTE_HANDLER;
}

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效  
void DisableSetUnhandledExceptionFilter()
{
	void* addr = (void*)GetProcAddress(LoadLibrary("kernel32.dll"),
		"SetUnhandledExceptionFilter");

	if (addr)
	{
		unsigned char code[16];
		int size = 0;

		code[size++] = 0x33;
		code[size++] = 0xC0;
		code[size++] = 0xC2;
		code[size++] = 0x04;
		code[size++] = 0x00;

		DWORD dwOldFlag, dwTempFlag;
		VirtualProtectEx(GetCurrentProcess(), addr, size, PAGE_EXECUTE_READWRITE, &dwOldFlag);
		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
		VirtualProtectEx(GetCurrentProcess(), addr, size, dwOldFlag, &dwTempFlag);
	}
}

void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	throw EX_CRT_INVALID_PARAM;
}

void PurecallHandler()
{
	throw EX_CRT_PURECALL;
}

int ProgramMemoryDepletionHander(size_t uSize)
{
	throw EX_CRT_NEW;
}

void InitMinDump()
{
	//注册异常处理函数  
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	//使SetUnhandledExceptionFilter  
	DisableSetUnhandledExceptionFilter();

	//CRT 错误处理
	_set_invalid_parameter_handler(InvalidParameterHandler);
	_set_purecall_handler(PurecallHandler);
	_set_new_handler(ProgramMemoryDepletionHander);
}

// 处理命令
void _HandleCommand(const std::string& command)
{
	std::cout << "命令:" << command << std::endl;
}

int main()
{
	std::cout << "logonserver Begin...\n";

	InitMinDump();

	// 初始化随机数种子
	srand((unsigned)time(NULL));

	// 设置程序路径
	CINIFile::GetAppPath();

	CUtil::MkdirIfNotExists("log");
	CUtil::MkdirIfNotExists("C:\\HM-Dump-v1.0");

	// 设置服务器类型
	ConfigManage()->SetServiceType(SERVICE_TYPE_LOGON);

	// 关联大厅主线程的log文件
	GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);

	INFO_LOG("================================================================================");
	INFO_LOG("======================logonserver begin========================================");
	INFO_LOG("================================================================================");

	bool ret = false;

	// 加载基础配置
	ret = ConfigManage()->Init();
	if (!ret)
	{
		ERROR_LOG("load config failed");
		std::cout << "ConfigManage::Init error!\n";
		system("pause");
		return -1;
	}

	ManageInfoStruct Init;
	memset(&Init, 0, sizeof(Init));

	// 初始化服务
	if (!g_LogonServerModule.InitService(&Init))
	{
		ERROR_LOG("InitService failed");
		std::cout << "InitService Error !\n";
		system("pause");
		return -1;
	}

	// 启动服务
	UINT errCode = 0;
	if (!g_LogonServerModule.StartService(errCode))
	{
		printf("Start Service Failed ,Error Code:%X\n", errCode);
		system("pause");
		return -1;
	}

	char szBuf[256] = "";
	sprintf(szBuf, "LogonServer启动成功 [Port:%d] [LogonID:%d] [MaxPeople:%d]", 
		g_LogonServerModule.m_LogonManage.m_nPort, ConfigManage()->m_logonServerConfig.logonID,
		g_LogonServerModule.m_LogonManage.m_uMaxPeople);
	std::cout << szBuf << std::endl;

	// 标题（显示版本信息）
	printf("v%d.%d.%d %s  ", VER_MAIN, VER_MIDDLE, VER_RESVERSE, VER_BUILDTIME);

	// 输出时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	printf("[%04d/%02d/%02d-%02d:%02d:%02d]\n", time.wYear, time.wMinute, time.wDay, time.wHour, time.wMinute, time.wSecond);

	std::cout << "输入 [exit] 退出\n";

	std::string command;

	while (true)
	{
		std::cout << "$";
		std::cin >> command;
		if (command == "exit" || command == "quit")
		{
			break;
		}
		_HandleCommand(command);
	}

	g_LogonServerModule.StoptService();

	INFO_LOG("========================================================================");
	INFO_LOG("==========================Logonserver end================================");
	INFO_LOG("========================================================================");

	GameLogManage()->Release();
	ConfigManage()->Release();

	_CrtDumpMemoryLeaks();

	std::cout << "Logonserver end\n";
	system("pause");

	return 0;
}