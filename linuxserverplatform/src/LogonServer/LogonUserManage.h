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

	LogonUserInfo()
	{
		userID = 0;
		socketIdx = -1;
		isVirtual = 0;
		roomID = 0;
		pBufferevent = nullptr;
		logonTime = 0;
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
	bool AddUser(int userID, LogonUserInfo* pUser);
	void DelUser(int userID);
	void Release();
	std::unordered_map<int, LogonUserInfo*>& GetLogonUserInfoMap() { return m_logonUserInfoMap; }
	UINT GetUserCount() { return m_logonUserInfoMap.size(); }

private:
	std::unordered_map<int, LogonUserInfo*> m_logonUserInfoMap;			// 大厅玩家管理器中只保存在线的玩家
};
