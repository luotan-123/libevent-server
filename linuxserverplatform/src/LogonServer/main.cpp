#include "CommonHead.h"
#include "GameLogonManage.h"
#include "GameLogonModule.h"

#define APP_TITLE "Gate-Logon-Server"

// 命令模式
//#define COMMAND_MODE

// 主进程退出开关
bool g_mainProcess = true;

// 登陆服务器（网关服务器）模块
CGameLogonModule	g_LogonServerModule;

// 处理命令
void _HandleCommand(const std::string& command)
{
	std::cout << "命令:[" << command << "]" << std::endl;
}

int main()
{
	// 设置程序路径 , 创建日志目录
	CINIFile file(CINIFile::GetAppPath() + "config.ini");
	string logPath = file.GetKeyVal("COMMON", "logPath", "./log/");
	if (!CUtil::MkdirIfNotExists(logPath.c_str()))
	{
		printf("创建日志目录失败！！！ err=%s", strerror(errno));
		return -1;
	}
	GameLogManage()->SetLogPath(logPath);

	// 生成core文件目录
	CUtil::MkdirIfNotExists(SAVE_COREFILE_PATH);

	// 设置服务器类型
	ConfigManage()->SetServiceType(SERVICE_TYPE_LOGON);

	// 关联大厅主线程的log文件
	GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);

	std::cout << std::endl;
	
	CON_INFO_LOG("======================%s begin========================================", APP_TITLE);

	bool ret = false;

	// 加载基础配置
	ret = ConfigManage()->Init();
	if (!ret)
	{
		CON_ERROR_LOG("ConfigManage::Init error! view log file !!!");
		return -1;
	}

	ManageInfoStruct Init;
	memset(&Init, 0, sizeof(Init));

	// 初始化服务
	if (!g_LogonServerModule.InitService(&Init))
	{
		CON_ERROR_LOG("InitService Error ! view log file !!! ");
		return -1;
	}

	// 启动服务
	UINT errCode = 0;
	if (!g_LogonServerModule.StartService(errCode))
	{
		CON_ERROR_LOG("Start Service Failed ,Error Code:%X . view log file !!!\n", errCode);
		return -1;
	}

	int logonID = ConfigManage()->GetLogonServerConfig().logonID;
	LogonBaseInfo* pLogonBaseInfo = ConfigManage()->GetLogonBaseInfo(logonID);
	if (pLogonBaseInfo)
	{
		char szBuf[256] = "";
		sprintf(szBuf, "LogonServer启动成功 [LogonID:%d] [tcpPort:%d] [tcpMaxPeople:%d] [wbPort:%d] [wbMaxPeople:%d]",
			ConfigManage()->m_logonServerConfig.logonID, pLogonBaseInfo->port, pLogonBaseInfo->maxPeople,
			pLogonBaseInfo->webSocketPort, pLogonBaseInfo->maxWebSocketPeople);
		std::cout << szBuf << std::endl;
	}

	// 标题（显示版本信息和进程id）
	printf("v%d.%d.%d %s  processID:%d\n", VER_MAIN, VER_MIDDLE, VER_RESVERSE, VER_BUILDTIME, getpid());

#ifdef COMMAND_MODE

	std::cout << "输入 [exit] 退出\n";

	std::string command;

	while (true)
	{
		std::cout << "$ ";
		std::getline(std::cin, command);
		if (command == "exit" || command == "quit")
		{
			break;
		}
		_HandleCommand(command);
	}

#else

	CON_INFO_LOG("======================%s 启动成功========================================", APP_TITLE);

	select(0, 0, 0, 0, 0);
	/*while (g_mainProcess)
	{
		sleep(60);
	}*/

#endif

	g_LogonServerModule.StoptService();

	CON_INFO_LOG("==========================Logonserver end================================");

	GameLogManage()->Release();
	ConfigManage()->Release();

	exit(EXIT_SUCCESS);

	return 0;
}