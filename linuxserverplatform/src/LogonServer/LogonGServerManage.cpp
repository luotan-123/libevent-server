#include "CommonHead.h"
#include "log.h"
#include "Function.h"
#include "LogonGServerManage.h"

CLogonGServerManage::CLogonGServerManage()
{
}

CLogonGServerManage::~CLogonGServerManage()
{
	Release();
}

LogonGServerInfo * CLogonGServerManage::GetGServer(int roomID)
{
	auto iter = m_logonGServerInfoMap.find(roomID);
	if (iter != m_logonGServerInfoMap.end())
	{
		return iter->second;
	}

	return NULL;
}

bool CLogonGServerManage::AddGServer(int roomID, LogonGServerInfo* pGServer)
{
	if (roomID <= 0 || !pGServer)
	{
		return false;
	}

	if (m_logonGServerInfoMap.find(roomID) != m_logonGServerInfoMap.end())
	{
		return false;
	}

	m_logonGServerInfoMap.insert(std::make_pair(roomID, pGServer));

	return true;
}

void CLogonGServerManage::DelGServer(int roomID)
{
	auto iter = m_logonGServerInfoMap.find(roomID);
	if (iter == m_logonGServerInfoMap.end())
	{
		ERROR_LOG("find gserver failed roomID:%d", roomID);
		return;
	}

	SAFE_DELETE(iter->second);
	m_logonGServerInfoMap.erase(iter);
}

void CLogonGServerManage::Release()
{
	for (auto iter = m_logonGServerInfoMap.begin(); iter != m_logonGServerInfoMap.end(); iter++)
	{
		SAFE_DELETE(iter->second);
	}

	m_logonGServerInfoMap.clear();
}
