#pragma once

#include <map>
#include <unordered_map>

// 大厅gserver结构
struct LogonGServerInfo
{
	int roomID;
	int socketIdx;
	
	LogonGServerInfo()
	{
		roomID = 0;
		socketIdx = -1;
	}
};

// 大厅gserver管理器
class CLogonGServerManage
{
public:
	CLogonGServerManage();
	~CLogonGServerManage();

public:
	LogonGServerInfo* GetGServer(int roomID);
	bool AddGServer(int roomID, LogonGServerInfo* pGServer);
	void DelGServer(int roomID);
	void Release();
	std::unordered_map<int, LogonGServerInfo*>& GetLogonGServerInfoMap() { return m_logonGServerInfoMap; }
	UINT GetGServerCount() { return m_logonGServerInfoMap.size(); }

private:
	std::unordered_map<int, LogonGServerInfo*> m_logonGServerInfoMap;
};
