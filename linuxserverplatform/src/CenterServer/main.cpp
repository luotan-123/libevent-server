#include "CommonHead.h"
#include "CenterServerManage.h"
#include "CenterServerModule.h"

#define APP_TITLE "中心服CenterServer"
#define APP_CENTER_TITLE	"CenterServer"

// 命令模式
//#define COMMAND_MODE

// 中心服务器模块
CCenterServerModule		g_CenterServerModule;

// 处理命令
void _HandleCommand(const std::string& command)
{
	std::cout << "命令:[" << command << "]" << std::endl;
}

int main()
{
	// 初始化随机数种子
	srand((unsigned)time(NULL));

	//// 查看实例是否唯一
	//if (GetProcessPidByName(APP_CENTER_TITLE) != -1)
	//{
	//	std::cout << "centerserver already exists\n";
	//	std::cout << "按下一个键退出\n";
	//	getchar();
	//	return -1;
	//}

	// 设置程序路径
	//CINIFile::GetAppPath();

	CUtil::MkdirIfNotExists("log");
	CUtil::MkdirIfNotExists("/tmp/web/");
	CUtil::MkdirIfNotExists("/tmp/web/json");

	// 设置服务器类型
	ConfigManage()->SetServiceType(SERVICE_TYPE_CENTER);

	// 关联大厅主线程的log文件
	GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);

	std::cout << std::endl;
	CON_INFO_LOG("================================================================================");
	CON_INFO_LOG("======================centerserver begin========================================");
	CON_INFO_LOG("================================================================================");

	bool ret = false;

	// 加载基础配置
	ret = ConfigManage()->Init();
	if (!ret)
	{
		ERROR_LOG("load config failed");
		std::cout << "ConfigManage::Init error!\n";
		std::cout << "按下一个键退出\n";
		getchar();
		return -1;
	}

	ManageInfoStruct Init;
	memset(&Init, 0, sizeof(Init));

	// 初始化服务
	if (!g_CenterServerModule.InitService(&Init))
	{
		ERROR_LOG("InitService failed");
		std::cout << "InitService Error !\n";
		std::cout << "按下一个键退出\n";
		getchar();
		return -1;
	}

	// 启动服务
	UINT errCode = 0;
	if (!g_CenterServerModule.StartService(errCode))
	{
		printf("Start Service Failed ,Error Code:%X\n", errCode);
		std::cout << "按下一个键退出\n";
		getchar();
		return -1;
	}

	char szBuf[256] = "";
	sprintf(szBuf, "中央集群服务系统 启动成功 [Port:%d] [MaxPeople:%d]", g_CenterServerModule.m_CenterServerManage.m_nPort
	, g_CenterServerModule.m_CenterServerManage.m_uMaxPeople);
	std::cout << szBuf << std::endl;

	// 标题（显示版本信息和进程id）
	printf("v%d.%d.%d %s  processID:%d\n", VER_MAIN, VER_MIDDLE, VER_RESVERSE, VER_BUILDTIME, getpid());

#ifdef COMMAND_MODE

	std::cout << "输入 [exit] 退出\n";

	string command = "";

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

	CON_INFO_LOG("======================CenterServer 启动成功========================================");

	while (true)
	{
		sleep(600);
	}

#endif

	g_CenterServerModule.StoptService();

	CON_INFO_LOG("========================================================================");
	CON_INFO_LOG("==========================CenterServer end================================");
	CON_INFO_LOG("========================================================================");

	GameLogManage()->Release();
	ConfigManage()->Release();

	exit(EXIT_SUCCESS);

	return 0;
}