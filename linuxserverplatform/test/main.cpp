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
#include "test1.pb.h"
#include "ServerTimer.h"
#include "GameMainManage.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace test;
using namespace test1;

static timeval g_lasttime;
void* TimerFun(void* p)
{
	evutil_gettimeofday(&g_lasttime, NULL);
	CDataLine* pDataLine = (CDataLine*)p;
	//数据缓存
	BYTE szBuffer[8192] = "";
	DataLineHead* pDataLineHead = (DataLineHead*)szBuffer;

	while (true)
	{
		usleep(20000);

		while (pDataLine->GetDataCount())
		{
			unsigned int bytes = pDataLine->GetData(&pDataLineHead);

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

class FIFOEvent
{
public:
	FIFOEvent(const char* filename);
	~FIFOEvent(){}
	void WaitForEvent();
	void SetEvent();
private:
	const char* m_fifoName;
};
FIFOEvent::FIFOEvent(const char* filename)
{
	m_fifoName = filename;

	int res = 0;

	if (access(m_fifoName, F_OK) == -1)
	{
		res = mkfifo(m_fifoName, 0777);
		if (res != 0)
		{
			printf("could not create fifo:%s\n", strerror(errno));
			m_fifoName = NULL;
		}
	}
}
void FIFOEvent::WaitForEvent()
{
	if (!m_fifoName)
	{
		return;
	}

	int fifoFd = open(m_fifoName, O_RDONLY);
	int res = 0;

	if (fifoFd != -1)
	{
		char chTemp;
		res = read(fifoFd, &chTemp, sizeof(chTemp));
		if (res == -1)
		{
			printf("read error:%s\n", strerror(errno));
		}

		close(fifoFd);
	}
	else
	{
		printf("open error:%s\n", strerror(errno));
	}
}
void FIFOEvent::SetEvent()
{
	if (!m_fifoName)
	{
		return;
	}

	int fifoFd = open(m_fifoName, O_WRONLY);
	int res = 0;

	if (fifoFd != -1)
	{
		char chTemp = 123;
		res = write(fifoFd, &chTemp, sizeof(chTemp));
		if (res == -1)
		{
			printf("write error:%s\n", strerror(errno));
		}

		close(fifoFd);
	}
	else
	{
		printf("open error:%s\n", strerror(errno));
	}
}

void* FIFOFunc(void*param)
{
	printf("测试管道\n");
	
	FIFOEvent* p = (FIFOEvent*)param;

	p->SetEvent();
}

int main()
{
	while (true)
	{
		sleep(1);

		int switch_on = CUtil::GetRandNum()%2;

		switch (switch_on)
		{
		case 1:
			printf("======\n");
			break;
		default:
			continue;
			break;
		}

		printf("+++++++++++++++++ %d\n", switch_on);
	}



	//FIFOEvent fifo("/tmp/linuxserver-main-fifo");

	//// 开辟线程
	//pthread_t threadID2 = 0;
	//pthread_create(&threadID2, NULL, FIFOFunc, (void*)&fifo);
	//fifo.WaitForEvent();


	printf("罗潭\n");
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	Team team;
	team.set_id(1);
	team.set_name("Rocket");
	Student* s1 = team.add_student(); // 添加repeated成员
	s1->set_id(1);
	s1->set_name("Mike");
	s1->set_sex(Sex::BOY);
	Student* s2 = team.add_student();
	s2->set_id(2);
	s2->set_name("Lily");
	s2->set_sex(Sex::GIRL);

	// encode --> bytes stream
	string out;
	team.SerializeToString(&out);

	Team1 ttt;
	ttt.ParseFromArray(out.c_str(), out.size()); // or parseFromString
	cout << ttt.DebugString() << endl;
	for (int i = 0; i < ttt.student_size(); i++) {
		Student1 s = ttt.student(i); // 按索引解repeated成员
		cout << s.name() << " " << s.sex() << endl;
	}

	// decode --> team structure
	Team t;
	t.ParseFromArray(out.c_str(), out.size()); // or parseFromString
	cout << t.DebugString() << endl;
	for (int i = 0; i < t.student_size(); i++) {
		Student s = t.student(i); // 按索引解repeated成员
		cout << s.name() << " " << s.sex() << endl;
	}





	//CUtil::MkdirIfNotExists("log/");
	//CUtil::MkdirIfNotExists(SAVE_JSON_PATH);
	//// 设置服务器类型
	//ConfigManage()->SetServiceType(SERVICE_TYPE_LOADER);
	//// 关联大厅主线程的log文件
	//GameLogManage()->AddLogFile(GetCurrentThreadId(), THREAD_TYPE_MAIN);
	//ConfigManage()->Init();



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



	//发送邮件接口
	MyCurl curl;
	std::vector<std::string> vUrlHeader;
	std::string postFields = "";
	std::string result = "";
	//组合生成URL
	std::string url = "http://api.androidhive.info/volley/person_object.json";
	curl.postUrlHttps(url, vUrlHeader, postFields, result);
	std::cout << result << endl;



	string strJsonContent = "{\"role_id\": 1,\"occupation\": \"paladin\",\"camp\": \"alliance\"}";
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
	cout << "camp is: " << strCamp << endl;


	LogonResponseLogon msg;
	strcpy(msg.name, "123456");
	msg.money = 963852741;
	Xor::Encrypt((unsigned char *)&msg, sizeof(msg));
	Xor::Decrypt((unsigned char*)& msg, sizeof(msg));

	

	printf("+++++++++++++++==\n"); 
	CSignedLock lock;
	CSignedLockObject testLock(&lock, false);

	testLock.Lock();
	testLock.Lock();
	printf("+++++++++++++++==\n");

	testLock.UnLock();
	testLock.UnLock();

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