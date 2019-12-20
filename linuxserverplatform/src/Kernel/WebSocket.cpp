#include "CommonHead.h"
#include "basemessage.h"
#include "log.h"
#include "configManage.h"
#include "DataLine.h"
#include "Xor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "WebSocket.h"

//////////////////////////////////////////////////////////////////////
CWebSocketManage::CWebSocketManage()
{
	
}

CWebSocketManage::~CWebSocketManage()
{

}

bool CWebSocketManage::SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, void* pBufferevent, unsigned int uIdentification/* = 0*/)
{
	if (!IsConnected(index))
	{
		ERROR_LOG("socketIdx close, index=%d, mainID=%d assistID=%d", index, mainID, assistID);
		return false;
	}

	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(NetMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

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

	// 数据加密
	if (encrypted)
	{
		Xor::Encrypt((uint8_t*)(buf + sizeof(NetMessageHead)), size);
	}

	// 投递到发送队列
	if (m_pSendDataLine)
	{
		SendDataLineHead lineHead;
		lineHead.socketIndex = index;
		lineHead.pBufferevent = pBufferevent;
		unsigned int addBytes = m_pSendDataLine->AddData(&lineHead.dataLineHead, sizeof(lineHead), 0, buf, pHead->uMessageSize);

		if (addBytes == 0)
		{
			ERROR_LOG("投递消息失败,mainID=%d,assistID=%d", mainID, assistID);
			return false;
		}
	}

	return true;
}

bool CWebSocketManage::HandleData(bufferevent* bev, int index)
{
	if (bev == NULL)
	{
		ERROR_LOG("HandleData error bev == NULL");
		return false;
	}

	struct evbuffer* input = bufferevent_get_input(bev);

	size_t maxSingleRead = Min_(evbuffer_get_length(input), SOCKET_RECV_BUF_SIZE);

	char buf[maxSingleRead];

	size_t realAllSize = evbuffer_copyout(input, buf, sizeof(buf));
	if (realAllSize <= 0)
	{
		return false;
	}

	// 剩余处理数据
	size_t handleRemainSize = realAllSize;

	// 解出包头
	NetMessageHead* pNetHead = (NetMessageHead*)buf;

	// 错误判断
	if (handleRemainSize >= sizeof(NetMessageHead) && pNetHead->uMessageSize > SOCKET_RECV_BUF_SIZE)
	{
		// 消息格式不正确
		CloseSocket(index);
		ERROR_LOG("消息格式不正确,index=%d", index);
		return false;
	}

	// 粘包处理
	while (handleRemainSize >= sizeof(NetMessageHead) && handleRemainSize >= pNetHead->uMessageSize)
	{
		unsigned int messageSize = pNetHead->uMessageSize;
		if (messageSize > MAX_TEMP_SENDBUF_SIZE)
		{
			// 消息格式不正确
			CloseSocket(index);
			ERROR_LOG("消息格式不正确");
			return false;
		}

		int realSize = messageSize - sizeof(NetMessageHead);
		if (realSize < 0)
		{
			// 数据包不够包头
			CloseSocket(index);
			ERROR_LOG("数据包不够包头");
			return false;
		}

		void* pData = NULL;
		if (realSize > 0)
		{
			// 没数据就为NULL
			pData = (void*)(buf + realAllSize - handleRemainSize + sizeof(NetMessageHead));
		}

		// 派发数据
		DispatchPacket(bev, index, pNetHead, pData, realSize);

		handleRemainSize -= messageSize;

		pNetHead = (NetMessageHead*)(buf + realAllSize - handleRemainSize);
	}

	evbuffer_drain(input, realAllSize - handleRemainSize);

	return true;
}
