#pragma once

#include "RedisCommon.h"
#include <vector>
#include <string>
#include "configManage.h"
#include "NewMessageDefine.h"

class CRedisLogon : public CRedisCommon
{
public:
	CRedisLogon();
	virtual ~CRedisLogon();

public:
	// 初始化
	virtual bool Init();
	// 关闭
	virtual bool Stop();
	// 获取redis Context
	redisContext* GetRedisContext();
public:
	// 注册用户
	int  Register(const UserData& userData, BYTE registerType);

	// 购买桌子相关
	int  GetDeskMixIDByPasswd(const char* passwd);
	std::string CreatePrivateDeskRecord(int userID, int roomID, BYTE masterNotPlay, int maxDeskCount, int buyGameCount,
		const char* pGameRules, int maxUserCount, int payType, int maxWatchUserCount, int	friendsGroupID, int friendsGroupDeskNumber);
	int  GetMarkDeskIndex(int roomID);

	//工具函数
	bool SetUserInfo(int userID, std::string strLine);
	int GetRelevanceTrdUid(std::string triID);	// 第三方登录获取userID
	int GetVisitorID(std::string triID);		// 客登录获取userID
	bool ClearAllUserWinCount();
	void AddUserBuyDeskSet(int userID, const std::string& passwd);
	std::string GetRandUnRepeatedDeskPasswd();
	bool ClearAllUserRanking(const char* ranking);


	//奖池操作
	bool ClearOneDayWinMoney();
	bool CountOneDayPoolInfo();

	///////////////////////登陆注册/////////////////////////////
	// 手机号登录
	int GetUserIDByPhone(const char* phone);
	// 获取当前最大id
	int  GetCurMaxUserID();
	// 闲聊登陆
	int GetUserIDByXianLiao(const char* xianliao);

	// 设置登录服人数
	bool SetLogonServerCurrPeopleCount(int logonID, int peopleCount);

	///////////////////////////////////战绩模块///////////////////////////////////////
	bool GetGradeSimpleInfo(long long id, PrivateDeskGradeSimpleInfo& simpleInfoVec);
	bool GetGradeDetailInfo(long long id, GameGradeInfo& gameGradeInfo);
	void ClearGradeInfo();
	void ClearUserGradeInfo(int userID);
	void ClearGradeSimpleInfoByRoomID(int roomID);
	void ClearGradeDetailInfoByRoomID(int roomID);

	///////////////////////////////////俱乐部房间模块///////////////////////////////////////
	int IsCanCreateFriendsGroupRoom(int userID, int friendsGroupID, BYTE userPower, int friendsGroupDeskNumber);
	bool CreateFriendsGroupDeskInfo(int friendsGroupID, int friendsGroupDeskNumber, const std::string &deskPasswd,
		int gameID, int roomType, OneFriendsGroupDeskInfo &deskInfo, bool &bHaveRedSpot);
	int GetFGDeskMixID(const char * asskey);
	bool GetTempFgDesk(const char * asskey, SaveRedisFriendsGroupDesk & desk);  //恢复牌桌用到
	int GetAllTempFgDesk(std::vector<SaveRedisFriendsGroupDesk>& vecFGDesk);    //恢复牌桌用到
public:
	// 保存redis中的数据到DB
	void RountineSaveRedisDataToDB(bool updateAll);
	bool SaveRedisDataToDB(const char* key, const char* tableName, int id, int mode);

	//分布式
	bool IsMainDistributedSystem(); //判断是否是当前主要集群系统
	bool IsDistributedSystemCalculate(long long calcID); //这个id是否当前系统计算
public:
	//分布式处理相关
	UINT m_uLogonGroupIndex;		//登陆服集群索引
	UINT m_uLogonGroupCount;		//登陆服集群数量
	UINT m_uMainLogonGroupIndex;	//当前主要登陆服集群索引（相比其它服务器集群处理更多数据）
private:
	std::vector<FieldRealInfo> m_vecFields;
};