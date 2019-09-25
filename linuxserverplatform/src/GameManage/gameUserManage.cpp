#include "GameMainManage.h"
#include "gameUserManage.h"

CGameUserManage::CGameUserManage()
{

}

CGameUserManage::~CGameUserManage()
{
	Release();
}

void CGameUserManage::Release()
{
	for (auto iter = m_gameUserInfoMap.begin(); iter != m_gameUserInfoMap.end(); ++iter)
	{
		SAFE_DELETE(iter->second);
	}

	m_gameUserInfoMap.clear();
}

bool CGameUserManage::Init()
{
	return true;
}

bool CGameUserManage::UnInit()
{
	Release();

	return true;
}

GameUserInfo* CGameUserManage::GetUser(int userID)
{
	if (userID <= 0)
	{
		return NULL;
	}

	try
	{
		auto iter = m_gameUserInfoMap.find(userID);
		if (iter != m_gameUserInfoMap.end())
		{
			return iter->second;
		}

		return NULL;
	}
	catch (...)
	{
		ERROR_LOG("GetUser catch expection!!!! size=%d", m_gameUserInfoMap.size());
	}

	return NULL;
}

bool CGameUserManage::AddUser(GameUserInfo* pUser)
{
	if (!pUser)
	{
		return false;
	}

	if (pUser->userID <= 0)
	{
		return false;
	}

	if (m_gameUserInfoMap.find(pUser->userID) != m_gameUserInfoMap.end())
	{
		return false;
	}

	m_gameUserInfoMap.insert(std::make_pair(pUser->userID, pUser));

	return true;
}

void CGameUserManage::DelUser(int userID)
{
	auto iter = m_gameUserInfoMap.find(userID);
	if (iter == m_gameUserInfoMap.end())
	{
		ERROR_LOG("find user failed userID:%d", userID);
		return;
	}

	SAFE_DELETE(iter->second);
	m_gameUserInfoMap.erase(iter);
}

bool CGameUserManage::IsUserOnLine(int userID)
{
	auto iter = m_gameUserInfoMap.find(userID);
	if (iter != m_gameUserInfoMap.end())
	{
		GameUserInfo* pUser = iter->second;
		if (pUser)
		{
			return pUser->IsOnline;
		}
	}

	return false;
}

void CGameUserManage::CheckInvalidStatusUser(CGameMainManage* pManage)
{
	if (!pManage)
	{
		return;
	}

	CRedisLoader* pRedis = pManage->GetRedis();
	if (!pRedis)
	{
		return;
	}

	auto iter = m_gameUserInfoMap.begin();
	for (; iter != m_gameUserInfoMap.end();)
	{
		GameUserInfo* pUser = iter->second;
		if (!pUser)
		{
			++iter;
			continue;
		}

		if (pUser->playStatus != USER_STATUS_STAND)
		{
			++iter;
			continue;
		}

		// 被移除前，玩家必须不在桌子的任意位置上
		if (pUser->deskIdx != INVALID_DESKIDX && pUser->deskStation != INVALID_DESKSTATION)
		{
			++iter;
			continue;
		}

		INFO_LOG("清理无效玩家内存数据 userID=%d deskIdx=%d deskStation=%d", pUser->userID, pUser->deskIdx, pUser->deskStation);

		SAFE_DELETE(pUser);
		m_gameUserInfoMap.erase(iter++);
	}
}

int CGameUserManage::GetOnlineUserCount()
{
	int count = 0;

	auto iter = m_gameUserInfoMap.begin();
	for (; iter != m_gameUserInfoMap.end(); ++iter)
	{
		GameUserInfo* pUser = iter->second;
		if (pUser && pUser->IsOnline == true)
		{
			count++;
		}
	}

	return count;
}

int CGameUserManage::GetUserCount()
{
	return m_gameUserInfoMap.size();
}