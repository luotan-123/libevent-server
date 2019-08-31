#pragma once

#include <map>
#include <unordered_map>
#include "Define.h"

// 玩家状态
enum UserStatus
{
	USER_STATUS_DEFAULT = 0,		// 默认状态
	USER_STATUS_WATCH,				// 旁观状态
	USER_STATUS_SITING,				// 坐下
	USER_STATUS_AGREE,				// 同意
	USER_STATUS_PLAYING,			// 游戏中
	USER_STATUS_STAND,				// 站起
};

// 游戏玩家信息
struct GameUserInfo
{
	int		userID;
	int		socketIdx;
	bool	IsOnline;						// 是否在线
	int		deskIdx;						// 玩家所在的deskIdx (在玩家坐下的时候设置)
	BYTE	deskStation;					// 玩家的座位
	int		playStatus;					    // 玩家状态
	long long  money;						// 玩家金币
	int		jewels;							// 玩家钻石
	char	name[MAX_USER_NAME_LEN];		// 玩家昵称
	char	headURL[MAX_USER_HEADURL_LEN];
	char	longitude[12];					// 经度
	char	latitude[12];					// 纬度
	char	address[64];					// 地址
	BYTE    userStatus;                     // 玩家身份
	char	ip[24];
	BYTE	isVirtual;						// 是否虚拟
	BYTE	sex;
	BYTE	choiceDeskStation;				// 玩家想选择的座位
	int		fireCoin;						// 玩家俱乐部火币
	char	motto[128];						// 个性签名
	time_t	lastOperateTime;				// 上次操作时间
	int		matchSocre;						// 比赛积分
	int		watchDeskIdx;					// 旁观的桌子

	GameUserInfo()
	{
		Clear();
	}

	void Clear()
	{
		memset(this, 0, sizeof(GameUserInfo));
		socketIdx = -1;
		deskIdx = INVALID_DESKIDX;
		deskStation = INVALID_DESKSTATION;
		choiceDeskStation = INVALID_DESKSTATION;
		lastOperateTime = 0;
	}
};

// 游戏玩家管理类, 管理一个游戏中玩家的信息
class CGameMainManage;
class KERNEL_CLASS CGameUserManage
{
public:
	CGameUserManage();
	~CGameUserManage();

	void Release();

public:
	bool Init();
	bool UnInit();

public:
	GameUserInfo* GetUser(int userID);
	bool AddUser(GameUserInfo* pUser);
	void DelUser(int userID);
	bool IsUserOnLine(int userID);
	void CheckInvalidStatusUser(CGameMainManage* pManage);
	int GetOnlineUserCount();
	int GetUserCount();

private:
	std::unordered_map<int, GameUserInfo*> m_gameUserInfoMap;
};