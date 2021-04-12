#include "LoaderServerModule.h"

// 登陆服务器（网关服务器）模块
CLoaderServerModule	g_LoaderServerModule;

// 命令模式
//#define COMMAND_MODE

// 主进程退出开关
bool g_mainProcess = true;

// 处理命令
void _HandleCommand(const std::string& command)
{
	std::cout << "命令:[" << command << "]" << std::endl;

	if (command == "update")
	{
		g_LoaderServerModule.UpdateAllRoom();
	}

}

int main()
{
	// 初始化随机数种子
	srand((unsigned)time(NULL));

	// 设置程序路径 , 创建日志目录
	CINIFile file(CINIFile::GetAppPath() + "config.ini");
	string logPath = file.GetKeyVal("COMMON", "logPath", "./log/");
	if (!CUtil::MkdirIfNotExists(logPath.c_str()))
	{
		printf("创建日志目录失败！！！ err=%s", strerror(errno));
		return -1;
	}
	GameLogManage()->SetLogPath(logPath);

	// 生成json目录和core文件目录
	CUtil::MkdirIfNotExists(SAVE_JSON_PATH);
	CUtil::MkdirIfNotExists(SAVE_COREFILE_PATH);

	// 设置服务器类型
	ConfigManage()->SetServiceType(SERVICE_TYPE_LOADER);

	// 关联大厅主线程的log文件
	GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);

	std::cout << std::endl;
	CON_INFO_LOG("======================Gameserver begin========================================");

	bool ret = false;

	// 加载基础配置
	ret = ConfigManage()->Init();
	if (!ret)
	{
		CON_ERROR_LOG("ConfigManage::Init error! 请查看启动日志 !!!");
		return -1;
	}

	// 启动游戏房间
	int iStartCount = 0, iFailCount = 0;
	g_LoaderServerModule.StartAllRoom(iStartCount, iFailCount);
	CON_INFO_LOG("启动成功房间数量：%d，失败数量：%d", iStartCount, iFailCount);
	if (iStartCount == 0)
	{
		CON_ERROR_LOG("没有找到任何游戏房间，请检查 roombaseinfo.serviceName名字是否正确，或者动态库名字配置错误");
		return -1;
	}

	// 标题（显示版本信息和进程id）
	printf("v%d.%d.%d %s  processID:%d\n", VER_MAIN, VER_MIDDLE, VER_RESVERSE, VER_BUILDTIME, getpid());

#ifdef COMMAND_MODE

	// 输出可用命令
	std::cout << "1.输入 [exit] 退出 \n2.输入 [update] 更新所有房间配置 \n";

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

	CON_INFO_LOG("======================Gameserver 启动成功========================================");

	select(0, 0, 0, 0, 0);
	/*while (g_mainProcess)
	{
		sleep(60);
	}*/

#endif

	g_LoaderServerModule.StopAllRoom();

	CON_INFO_LOG("==========================Gameserver end================================");

	GameLogManage()->Release();
	ConfigManage()->Release();

	exit(EXIT_SUCCESS);

	return 0;
}