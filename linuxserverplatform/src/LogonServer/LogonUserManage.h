#pragma once


// 大厅玩家结构
struct LogonUserInfo
{
	int userID;
	int socketIdx;
	BYTE isVirtual;
	int roomID;
	void* pBufferevent;
	time_t logonTime;
	BYTE socketType;

	LogonUserInfo()
	{
		userID = 0;
		socketIdx = -1;
		isVirtual = 0;
		roomID = 0;
		pBufferevent = nullptr;
		logonTime = 0;
		socketType = SOCKET_TYPE_TCP;
	}
};

// 大厅玩家管理器
class CLogonUserManage
{
public:
	CLogonUserManage();
	~CLogonUserManage();

public:
	LogonUserInfo* GetUser(int userID);

	// AddUser之前需要把socket类型传进来
	bool AddUser(int userID, LogonUserInfo* pUser);
	void DelUser(int userID);
	void Release();
	std::unordered_map<int, LogonUserInfo*>& GetLogonUserInfoMap() { return m_logonUserInfoMap; }
	UINT GetUserCount() { return m_logonUserInfoMap.size(); }
	UINT GetTcpSocketUserCount() { return m_tcpSocketCount; }
	UINT GetWebSocketUserCount() { return m_webSocketCount; }

private:
	std::unordered_map<int, LogonUserInfo*> m_logonUserInfoMap;			// 大厅玩家管理器中只保存在线的玩家
	UINT m_tcpSocketCount;
	UINT m_webSocketCount;
};
