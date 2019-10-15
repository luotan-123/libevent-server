#include "CommonHead.h"
#include "log.h"
#include "Function.h"
#include "LogonUserManage.h"

CLogonUserManage::CLogonUserManage()
{
}

CLogonUserManage::~CLogonUserManage()
{
	Release();
}

LogonUserInfo * CLogonUserManage::GetUser(int userID)
{
	auto iter = m_logonUserInfoMap.find(userID);
	if (iter != m_logonUserInfoMap.end())
	{
		return iter->second;
	}

	return NULL;
}

bool CLogonUserManage::AddUser(int userID, LogonUserInfo * pUser)
{
	if (userID <= 0 || !pUser)
	{
		return false;
	}

	if (m_logonUserInfoMap.find(userID) != m_logonUserInfoMap.end())
	{
		return false;
	}

	m_logonUserInfoMap.insert(std::make_pair(userID, pUser));

	return true;
}

void CLogonUserManage::DelUser(int userID)
{
	auto iter = m_logonUserInfoMap.find(userID);
	if (iter == m_logonUserInfoMap.end())
	{
		ERROR_LOG("find use failed userID:%d", userID);
		return;
	}

	SAFE_DELETE(iter->second);
	m_logonUserInfoMap.erase(iter);
}

void CLogonUserManage::Release()
{
	for (auto iter = m_logonUserInfoMap.begin(); iter != m_logonUserInfoMap.end(); iter++)
	{
		SAFE_DELETE(iter->second);
	}

	m_logonUserInfoMap.clear();
}
