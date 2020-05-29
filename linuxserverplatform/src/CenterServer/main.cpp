#include "CommonHead.h"
#include "CenterServerManage.h"
#include "CenterServerModule.h"
#include <fcntl.h>
#include <sys/stat.h>

#define LOCKFILE "/var/run/LinuxCenterServer.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define APP_CENTER_TITLE	"CenterServer"

// 命令模式
//#define COMMAND_MODE

// 主进程退出开关
bool g_mainProcess = true;

// 中心服务器模块
CCenterServerModule		g_CenterServerModule;

// 处理命令
void _HandleCommand(const std::string& command)
{
	std::cout << "命令:[" << command << "]" << std::endl;
}

/* set advisory lock on file */
int LockFile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;  /* write lock */
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;  //lock the whole file

	return(fcntl(fd, F_SETLK, &fl));
}

int AlreadyRunning(const char* filename)
{
	int fd;
	char buf[16];

	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0)
	{
		printf("can't open %s: %m\n", filename);
		return 0;
	}

	/* 先获取文件锁 */
	if (LockFile(fd) == -1)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			printf("file: %s already locked\n", filename);
			close(fd);
			return 1;
		}
		printf("can't lock %s: %m\n", filename);
		return 0;
	}

	/* 写入运行实例的pid */
	ftruncate(fd, 0);
	sprintf(buf, "%ld\n", (long)getpid());
	write(fd, buf, strlen(buf) + 1);

	return 0;
}

int main()
{
	// 判断一个程序是否已经运行
	// 根据需要调用下面函数
	/*if (AlreadyRunning(LOCKFILE))
	{
		CON_ERROR_LOG("centerserver already exists");
		return -1;
	}*/

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
	ConfigManage()->SetServiceType(SERVICE_TYPE_CENTER);

	// 关联大厅主线程的log文件
	GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);

	std::cout << std::endl;
	CON_INFO_LOG("======================centerserver begin========================================");

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
	if (!g_CenterServerModule.InitService(&Init))
	{
		CON_ERROR_LOG("InitService Error ! view log file !!! ");
		return -1;
	}

	// 启动服务
	UINT errCode = 0;
	if (!g_CenterServerModule.StartService(errCode))
	{
		CON_ERROR_LOG("Start Service Failed ,Error Code:%X . view log file !!!\n", errCode);
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

	select(0, 0, 0, 0, 0);
	/*while (g_mainProcess)
	{
		sleep(60);
	}*/

#endif

	g_CenterServerModule.StoptService();

	CON_INFO_LOG("==========================CenterServer end================================");

	GameLogManage()->Release();
	ConfigManage()->Release();

	exit(EXIT_SUCCESS);

	return 0;
}