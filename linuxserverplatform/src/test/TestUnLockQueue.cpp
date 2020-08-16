#include "CommonHead.h"
#include "log.h"
#include "NewMessageDefine.h"
#include "UnlockQueue.h"
#include "RRlockQueue.h"


//创建发送队列
RRlockQueue* pDataLine = new RRlockQueue();

// 消费者
void* consumer_proc(void* arg)
{

	printf("消费者:%lu\n", GetCurrentSysThreadId());

	//数据缓存
	//DataLineHead* pDataLineHead = NULL;
	DataLineHead* pDataLineHead = (DataLineHead*)malloc(MAX_SINGLE_UNLOCKQUEUE_SIZE);

	while (true)
	{
		try
		{
			//获取数据
			unsigned int bytes = pDataLine->GetData(&pDataLineHead);
			if (bytes == 0)
			{
				continue;
			}

			//处理数据
			SendDataLineHead* pSocketSend = (SendDataLineHead*)pDataLineHead;
			void* pBuffer = NULL;
			unsigned int size = pSocketSend->dataLineHead.uSize;

			if (size > sizeof(SendDataLineHead))
			{
				pBuffer = (void*)(pSocketSend + 1);			// 移动一个SendDataLineHead
				NetMessageHead* pHead = (NetMessageHead*)pBuffer;

				if (pHead->uMessageSize != 33 && pHead->uMessageSize != 44)
				{
					printf("出错\n");
				}
				

				//printf("addr = %p  sendsize=%d  uMainID=%u  uAssistantID=%u\n", pBuffer, size - sizeof(SendDataLineHead), pHead->uMainID, pHead->uAssistantID);

				
			}

			
		}
		catch (...)
		{
			ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
		}
	}

	INFO_LOG("CGServerConnect::ThreadSendMsg exit.");

	// 释放内存
	free(pDataLineHead);

	pthread_exit(NULL);
}

void SendData(int idx, void* pData, int size, int mainID, int assistID, int handleCode, unsigned int uIdentification)
{
	// 整合一下数据
	char buf[sizeof(NetMessageHead) + size];

	// 拼接包头
	NetMessageHead* pHead = (NetMessageHead*)buf;
	pHead->uMainID = mainID;
	pHead->uAssistantID = assistID;
	pHead->uMessageSize = sizeof(NetMessageHead) + size;
	pHead->uHandleCode = handleCode;
	pHead->uIdentification = uIdentification;

	// 包体
	if (pData && size > 0)
	{
		memcpy(buf + sizeof(NetMessageHead), pData, size);
	}


	SendDataLineHead lineHead;
	lineHead.socketIndex = idx;
	unsigned int addBytes = pDataLine->AddData(&lineHead.dataLineHead, sizeof(lineHead), 0, buf, pHead->uMessageSize);

	if (addBytes == 0)
	{
		ERROR_LOG("投递消息失败,mainID=%d,assistID=%d", mainID, assistID);
		printf("投递消息失败,mainID=%d,assistID=%d", pHead->uMainID, pHead->uAssistantID);
	}
}

// 生产者1
void* producer_proc1(void* arg)
{
	unsigned long pid = GetCurrentSysThreadId();
	printf("生产者:%lu\n", pid);

	while (true)
	{
		/*struct timespec slptm;
		slptm.tv_sec = 0;
		slptm.tv_nsec = 100000;
		nanosleep(&slptm, NULL);*/

		LogonRequestFriendsGroupAccoMsg msg;

		msg.endTime = 1234;
		msg.firstTime = 4321;

		SendData(0, &msg, sizeof(msg), pid, 2, 5, 1000);
	}

	pthread_exit(NULL);
}

// 生产者2
void* producer_proc2(void* arg)
{
	unsigned long pid = GetCurrentSysThreadId();
	printf("生产者:%lu\n", pid);

	while (true)
	{
		/*struct timespec slptm;
		slptm.tv_sec = 0;
		slptm.tv_nsec = 1000000;
		nanosleep(&slptm, NULL);*/

		LogonRequestCreateFriendsGroup msg;

		strcpy(msg.name, "罗潭程序员");

		SendData(0, &msg, sizeof(msg), pid, 11, 0, 3633);
	}

	pthread_exit(NULL);
}


int TestUnLockQueue()
{
	pthread_t consumer_tid, producer_tid;

	printf("multi thread test.......\n");

	if (0 != pthread_create(&producer_tid, NULL, producer_proc1, NULL))
	{
		fprintf(stderr, "Failed to create consumer thread.errno:%u, reason:%s\n",
			errno, strerror(errno));
		return -1;
	}

	if (0 != pthread_create(&producer_tid, NULL, producer_proc2, NULL))
	{
		fprintf(stderr, "Failed to create consumer thread.errno:%u, reason:%s\n",
			errno, strerror(errno));
		return -1;
	}

	if (0 != pthread_create(&consumer_tid, NULL, consumer_proc, NULL))
	{
		fprintf(stderr, "Failed to create consumer thread.errno:%u, reason:%s\n",
			errno, strerror(errno));
		return -1;
	}

	pthread_join(producer_tid, NULL);
	pthread_join(consumer_tid, NULL);

	return 0;
}