#include "LoaderServerModule.h"

// 登陆服务器（网关服务器）模块
CLoaderServerModule	g_LoaderServerModule;

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

	// 设置程序路径
	//CINIFile::GetAppPath();

	CUtil::MkdirIfNotExists("log");
	//CUtil::MkdirIfNotExists("C:\\HM-Dump-v1.0");

	// 设置服务器类型
	ConfigManage()->SetServiceType(SERVICE_TYPE_LOADER);

	// 关联大厅主线程的log文件
	GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);

	CON_INFO_LOG("================================================================================");
	CON_INFO_LOG("======================Gameserver begin========================================");
	CON_INFO_LOG("================================================================================");

	bool ret = false;

	// 加载基础配置
	ret = ConfigManage()->Init();
	if (!ret)
	{
		ERROR_LOG("load config failed");
		std::cout << "ConfigManage::Init error!\n按下一个键退出\n";
		getchar();
		return -1;
	}

	// 启动游戏房间
	int iStartCount = g_LoaderServerModule.StartAllRoom();
	CON_INFO_LOG("启动房间数量：%d", iStartCount);
	if (iStartCount == 0)
	{
		std::cout << "没有找到任何游戏房间，请检查 roombaseinfo.serviceName名字是否正确，或者动态库名字配置错误\n";
	}

	// 标题（显示版本信息）
	printf("v%d.%d.%d %s\n", VER_MAIN, VER_MIDDLE, VER_RESVERSE, VER_BUILDTIME);

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

	g_LoaderServerModule.StopAllRoom();

	CON_INFO_LOG("========================================================================");
	CON_INFO_LOG("==========================Gameserver end================================");
	CON_INFO_LOG("========================================================================");

	GameLogManage()->Release();
	ConfigManage()->Release();

	exit(EXIT_SUCCESS);

	return 0;
}