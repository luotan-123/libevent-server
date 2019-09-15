#pragma once

#include "hiredis.h"
#include "tableDefine.h"

// redis命令单词数量，实际上redis可以存储2^32-1个key-value
#define MAX_REDIS_CMD_WORD_COUNT	512		

//redis hash表扩展字段模式（定时扫描是根据对应的字段做相应的操作）
enum RedisExtendMode
{
	REDIS_EXTEND_MODE_DEFAULT = 0,
	REDIS_EXTEND_MODE_UPDATE,
	REDIS_EXTEND_MODE_INSERT,
	REDIS_EXTEND_MODE_DELETE,
	REDIS_EXTEND_MODE_END,
};

//redis 对应的key的类型
enum RedisKeyType
{
	REDIS_KEY_TYPE_NONE = 0,	// 无类型(错误)
	REDIS_KEY_TYPE_HASH,		// 哈希
};

class CDataBaseManage;
// CRedisBase 虚基类
class CRedisBase
{
public:
	CRedisBase();
	virtual ~CRedisBase();

public:
	// 初始化
	virtual bool Init() = 0;
	virtual bool Stop() = 0;

public:
	static std::string MakeKey(const char* tableName, long long id);
	// 合成redis中的key"tableName|id"形式
	static std::string MakeKey(const char* tableName, int id);
	// 合成redis中的key"tableName|id"形式
	static std::string MakeKey(const char* tableName, const char* id);
	// 解析redis中的key, "tableName|id"形式
	static bool ParseKey(const char* key, char* tableName, int& id);
	// 解析redis中的key, "tableName|id"形式
	static bool ParseKey(const char* key, char* tableName, long long& id);

public:
	// redis 中的key是否存在
	bool IsKeyExists(const char* key);
	// redis 中的key是否存在
	bool IsKeyExists(const char* mainKey, int mainID, int assID);
	// redis 中的key是否存在
	bool IsKeyExists(const char* mainKey, int mainID);
	// redis 中的key是否存在
	bool IsKeyExists(const char* mainKey, const char* assKey);
	// 删除指定的key
	bool DelKey(const char* key);
	// 获取key的类型
	int GetKeyType(const char* key);
	// 删除集合（无序集合）的元素
	bool SremMember(const char* key, const char* member);
	// 删除集合（有序集合）元素
	bool ZremMember(const char* key, const std::vector<long long>& memberVec);
	// 删除集合（有序集合）元素(int类型)
	bool ZremMember(const char* key, const std::vector<int>& memberVec);
	// 获取有序集合的大小
	int GetZSetSize(const char* key);
	// 设定超时时间
	bool SetExpire(const char* key, int secs);
	// 检查连接性
	void CheckConnection(const RedisConfig& redisConfig);
	// 测试redis读写性能
	void TestRedis(const char* name);
	// 获取某个键值索引
	long long GetKeyIndex(const char* key);
	// 设置某个键值索引
	bool SetKeyIndex(const char* key, long long llIndex);
	// 获取自增字符串
	long long GetIncrKeyIndex(const char* key);
	// 数据库
	void SetDBManage(CDataBaseManage* pDBManage);
protected:
	// 获取字段扩展模式
	int  GetExtendMode(const char* key);
	// 设置扩展模式
	bool SetExtendMode(const char* key, int mode);
	// 重连
	bool ReConnect(const RedisConfig& redisConfig);
	// 认证
	bool Auth(const char* passwd);
public:
	// 设置哈希表数据
	bool hmset(const char* table, int id, std::unordered_map<std::string, std::string>& fieldInfoUMap, int mode = REDIS_EXTEND_MODE_DEFAULT, const char* updateSet = NULL);
	bool hmset(const char* table, long long id, std::unordered_map<std::string, std::string>& fieldInfoUMap, int mode = REDIS_EXTEND_MODE_DEFAULT, const char* updateSet = NULL);
	bool hmset(const char* table, const char* id, std::unordered_map<std::string, std::string>& fieldInfoUMap, int mode = REDIS_EXTEND_MODE_DEFAULT, const char* updateSet = NULL);
public:
	int m_sleepTime;
protected:
	redisContext* m_pContext;
protected: //数据库相关
	CDataBaseManage* m_pDBManage;
};

// redis 锁
class CRedisLock
{
public:
	CRedisLock(redisContext* pContext, const char* key, int sleepTime = 0, bool bAutoLock = true);
	~CRedisLock();

public:
	void Lock();
	void Unlock();

private:
	redisContext* m_pContext;
	char m_key[128];
	int m_sleepTime;
};

// redis错误处理
#define REDIS_CHECKF(pReply, redisCmd)	{if (!pReply){ERROR_LOG("redisCommand return NULL redisCmd=%s context err=%d errstr=%s", redisCmd, m_pContext->err, m_pContext->errstr);return 0;}}
#define REDIS_CHECK(pReply, redisCmd)	{if (!pReply){ERROR_LOG("redisCommand return NULL redisCmd=%s context err=%d errstr=%s", redisCmd, m_pContext->err, m_pContext->errstr);return;}}
#define ASSERT_REDIS_REPLY_ERRORF(pReply, redisCmd)	{if (pReply->type==REDIS_REPLY_ERROR){ERROR_LOG("redisCommand return Error redisCmd=%s errstr=%s", redisCmd, pReply->str);freeReplyObject(pReply); return 0;}}
#define ASSERT_REDIS_REPLY_ERROR(pReply, redisCmd)	{if (pReply->type==REDIS_REPLY_ERROR){ERROR_LOG("redisCommand return Error redisCmd=%s errstr=%s", redisCmd, pReply->str);freeReplyObject(pReply);}}
#define ASSERT_REDIS_REPLY_NILF(pReply, redisCmd)	{if (pReply->type==REDIS_REPLY_NIL){ERROR_LOG("redisCommand return Error redisCmd=%s errstr=%s", redisCmd, pReply->str);freeReplyObject(pReply); return 0;}}
#define ASSERT_REDIS_REPLY_NIL(pReply, redisCmd)	{if (pReply->type==REDIS_REPLY_NIL){ERROR_LOG("redisCommand return Error redisCmd=%s errstr=%s", redisCmd, pReply->str);freeReplyObject(pReply);}}