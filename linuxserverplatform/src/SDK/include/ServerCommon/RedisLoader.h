#pragma once

#include "RedisCommon.h"

class CRedisLoader : public CRedisCommon
{
public:
	CRedisLoader();
	virtual ~CRedisLoader();

public:
	virtual bool Init();
	virtual bool Stop();

	//删除桌子
	bool DelPrivateDeskRecord(int deskMixID);
	void DelUserBuyDeskInfoInSet(int userID, const char* passwd);

	//记录人数信息
	bool SetPrivateDeskCurrUserCount(int deskMixID, int userCount);
	bool SetPrivateDeskCurrWatchUserCount(int deskMixID, int watchUserCount);

	//设置开房玩家列表
	bool SetPrivateDeskUserID(int deskMixID, int userID, int type);//0新增，1删除

	//胜局数相关
	bool SetUserWinCount(int userID);
	bool SetUserTotalGameCount(int userID);

	//设置桌子游戏局数
	bool SetPrivateDeskGameCount(int deskMixID, int gameCount);

	//生成战绩
	bool SetPrivateDeskGrade(GameGradeInfo& gameGradeInfo, long long& gradeID);
	bool SetPrivateDeskSimpleInfo(const std::vector<int>& userIDVec, PrivateDeskGradeSimpleInfo& simpleInfo, const std::vector<long long>& gradeIDVec);

	//机器人索引
	int GetRobotInfoIndex();

	//设置当前房间人数
	bool SetRoomServerPeopleCount(int roomID, int peopleCount);

	//关服保存桌子
	bool SaveFGDeskRoom(const PrivateDeskInfo &privateDeskInfo);

	//开服清理桌子
	bool CleanRoomAllData(int roomID);

	//获取房间所有创建的桌子
	bool GetAllDesk(int roomID, std::vector<int>& vecRooms);

	// 设置超时数据
	bool SetPrivateDeskCheckTime(int deskMixID, int checkTime);

	//获取玩家控制参数
	bool GetUserControlParam(int userID, int &value);

	//比赛场
	long long GetPartOfMatchIndex();
	bool DelFullPeopleMatchPeople(int gameID, int matchID, const std::vector<MatchUserInfo> &vecPeople);
	bool SetUserMatchStatus(int userID, BYTE matchType, int matchStatus);
	bool SetUserMatchRank(int userID, long long combineMatchID, int curMatchRank);
	bool SetTimeMatchPeopleRank(int matchID, const std::vector<MatchUserInfo> &vecPeople);
	int GetRobotUserID();
private:
	long long GetRoomMaxGradeIndex(int roomID);
	long long GetRoomMaxSimpleGradeIndex(int roomID);
};