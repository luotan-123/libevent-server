#include "CommonHead.h"
#include "Lock.h"
#include "basemessage.h"
#include "comstruct.h"
#include "KernelDefine.h"
#include "INIFile.h"
#include "configManage.h"
#include "Function.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "log.h"
#include "GServerConnect.h"
#include "DataLine.h"
#include "Util.h"
#include "MyCurl.h"
#include "Define.h"
#include "json/json.h"
#include "RedisCenter.h"
#include "Xor.h"
#include "NewMessageDefine.h"
#include "test.pb.h"
#include "ServerTimer.h"
#include "GameMainManage.h"


using namespace test;

static timeval g_lasttime;
void* TimerFun(void* p)
{
	evutil_gettimeofday(&g_lasttime, NULL);
	CDataLine* pDataLine = (CDataLine*)p;
	//数据缓存
	BYTE szBuffer[LD_MAX_PART] = "";
	DataLineHead* pDataLineHead = (DataLineHead*)szBuffer;

	while (true)
	{
		usleep(20000);

		while (pDataLine->GetDataCount())
		{
			unsigned int bytes = pDataLine->GetData(pDataLineHead, sizeof(szBuffer));

			ServerTimerLine* pTimerMessage = (ServerTimerLine*)pDataLineHead;

			/*if (pTimerMessage->uTimerID == 123456)
			{*/
				struct timeval newtime, difference;
				double elapsed;
				evutil_gettimeofday(&newtime, NULL);
				evutil_timersub(&newtime, &g_lasttime, &difference);
				elapsed = difference.tv_sec +
					(difference.tv_usec / 1.0e6);



				printf("timeout_cb called at %ld: %.3f seconds elapsed. timer %u \n",
					newtime.tv_sec, elapsed, pTimerMessage->uTimerID);
				g_lasttime = newtime;
			//}
			
		}

	}
}

int main()
{
	printf("罗潭\n");
	//system("sh start.sh");
	int a = 0;
	int b = 123456;
	unsigned long c = 123454;
	a = Min_(b, c);
	//GOOGLE_PROTOBUF_VERIFY_VERSION;

	//Team team;
	//team.set_id(1);
	//team.set_name("Rocket");
	//Student* s1 = team.add_student(); // 添加repeated成员
	//s1->set_id(1);
	//s1->set_name("Mike");
	//s1->set_sex(BOY);
	//Student* s2 = team.add_student();
	//s2->set_id(2);
	//s2->set_name("Lily");
	//s2->set_sex(GIRL);

	//// encode --> bytes stream
	//string out;
	//team.SerializeToString(&out);

	//// decode --> team structure
	//Team t;
	//t.ParseFromArray(out.c_str(), out.size()); // or parseFromString
	//cout << t.DebugString() << endl;
	//for (int i = 0; i < t.student_size(); i++) {
	//	Student s = t.student(i); // 按索引解repeated成员
	//	cout << s.name() << " " << s.sex() << endl;
	//}


	CUtil::MkdirIfNotExists("log/");
	CUtil::MkdirIfNotExists(SAVE_JSON_PATH);
	// 设置服务器类型
	ConfigManage()->SetServiceType(SERVICE_TYPE_LOADER);
	// 关联大厅主线程的log文件
	GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);
	ConfigManage()->Init();



	CGServerConnect ddd;
	CDataLine *dataline = new CDataLine;
	//ddd.Start(&dataline, 38);
	CServerTimer *pTime = new CServerTimer;
	pTime->Start(dataline);
	pTime->SetTimer(654321, 1700);
	pTime->SetTimer(123456, 2100, SERVERTIMER_TYPE_SINGLE);
	evutil_gettimeofday(&g_lasttime, NULL);
	printf("currtime %ld\n", g_lasttime.tv_sec);
	// 开辟线程
	pthread_t threadID1 = 0;
	int err = pthread_create(&threadID1, NULL, TimerFun, (void*)dataline);
	if (err != 0)
	{
		SYS_ERROR_LOG("ThreadCheckTimer failed");
	}



	//pTime->Stop();


	////发送邮件接口
	//MyCurl curl;
	//std::vector<std::string> vUrlHeader;
	//std::string postFields = "";
	//std::string result = "";
	////组合生成URL
	//std::string url = "http://api.androidhive.info/volley/person_object.json";
	//curl.postUrlHttps(url, vUrlHeader, postFields, result);
	//std::cout << result << endl;


	/*CRedisCenter* pRedis = new CRedisCenter;
	pRedis->Init();*/

	/*string strJsonContent = "{\"role_id\": 1,\"occupation\": \"paladin\",\"camp\": \"alliance\"}";
	int nRoleDd = 0;
	string strOccupation = "";
	string strCamp = "";
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(strJsonContent, root))
	{
		nRoleDd = root["role_id"].asInt();
		strOccupation = root["occupation"].asString();
		strCamp = root["camp"].asString();
	}
	cout << "role_id is: " << nRoleDd << endl;
	cout << "occupation is: " << strOccupation << endl;
	cout << "camp is: " << strCamp << endl;*/

	/*LogonResponseLogon msg;
	strcpy(msg.name, "123456");
	msg.money = 963852741;
	Xor::Encrypt((unsigned char *)&msg, sizeof(msg));
	Xor::Decrypt((unsigned char*)& msg, sizeof(msg));*/


	/*
	int fd = 10;
	int ret;
	ret = close(fd);
	if (ret == -1)
	{
		SYS_ERROR_LOG("close fd");
		CON_INFO_LOG("====");
		CON_ERROR_LOG("====");
	}*/
	

	

	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//printf("second:%ld\n", tv.tv_sec);  //秒
	//printf("millisecond:%ld\n", tv.tv_sec * 1000 + tv.tv_usec / 1000);  //毫秒
	//printf("microsecond:%ld\n", tv.tv_sec * 1000000 + tv.tv_usec);  //微秒

	//sleep(3); // 为方便观看，让程序睡三秒后对比
	//std::cout << "3s later:" << std::endl;

	//gettimeofday(&tv, NULL);
	//printf("second:%ld\n", tv.tv_sec);  //秒
	//printf("millisecond:%ld\n", tv.tv_sec * 1000 + tv.tv_usec / 1000);  //毫秒
	//printf("microsecond:%ld\n", tv.tv_sec * 1000000 + tv.tv_usec);  //微秒


	//printf("+++++++++++++++==\n"); 
	//CSignedLock lock;
	//CSignedLockObject testLock(&lock, false);

	//testLock.Lock();
	//testLock.Lock();
	//printf("+++++++++++++++==\n");

	//testLock.UnLock();
	//testLock.UnLock();

	int    socket_fd, connect_fd;
	struct sockaddr_in     servaddr;
	char    buff[4096];
	int     n;
	//初始化Socket  
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	//初始化  
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。  
	servaddr.sin_port = htons(6666);//设置的端口为DEFAULT_PORT  

	//将本地地址绑定到所创建的套接字上  
	if (bind(socket_fd, (struct sockaddr*) & servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	//开始监听是否有客户端连接  
	if (listen(socket_fd, 10) == -1) {
		printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}
	printf("======waiting for client's request======\n");
	while (1) {
		//阻塞直到有客户端连接，不然多浪费CPU资源。  
		if ((connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1) {
			printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
			continue;
		}
		//接受客户端传过来的数据  
		n = recv(connect_fd, buff, sizeof(buff), 0);
		//向客户端发送回应数据  
		if (!fork()) { /*紫禁城*/
			if (send(connect_fd, "Hello,you are connected!\n", 26, 0) == -1)
				perror("send error");
			close(connect_fd);
			exit(0);
		}
		buff[n] = '\0';
		printf("recv msg from client: %s\n", buff);
		close(connect_fd);
	}

	close(socket_fd);

    return 0;
}