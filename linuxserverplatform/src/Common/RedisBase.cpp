#include "CommonHead.h"
#include "configManage.h"
#include "log.h"
#include "comstruct.h"
#include "Interface.h"
#include "DataLine.h"
#include "DataBase.h"
#include "BillManage.h"
#include "Util.h"
#include "RedisBase.h"

CRedisBase::CRedisBase()
{
	m_sleepTime = 0;
	m_pContext = NULL;
	m_pDBManage = NULL;
}

CRedisBase::~CRedisBase()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

std::string CRedisBase::MakeKey(const char* tableName, int id)
{
	std::string key(tableName);
	key += '|';

	char buf[24] = "";
	sprintf(buf, "%d", id);
	key += buf;

	return key;
}

std::string CRedisBase::MakeKey(const char* tableName, const char* id)
{
	std::string key(tableName);
	key += '|';
	key += id;

	return key;
}

std::string CRedisBase::MakeKey(const char* tableName, long long id)
{
	std::string key(tableName);
	key += '|';

	char buf[24] = "";
	sprintf(buf, "%lld", id);
	key += buf;

	return key;
}

bool CRedisBase::ParseKey(const char* key, char* tableName, int& id)
{
	long long llID = 0;
	if (!ParseKey(key, tableName, llID))
	{
		return false;
	}

	id = (int)llID;

	return true;
}

// 解析redis中的key, "tableName|id"形式
bool CRedisBase::ParseKey(const char* key, char* tableName, long long& id)
{
	if (!key || !tableName)
	{
		return false;
	}

	int keySize = strlen(key);
	if (keySize <= 2)
	{
		return false;
	}

	char idStr[128] = "";
	int iSignPosition = -1;
	for (int i = 0; i < keySize; i++)
	{
		if (key[i] == '|')
		{
			iSignPosition = i;
			continue;
		}

		if (iSignPosition == -1)
		{
			tableName[i] = key[i];
		}
		else
		{
			idStr[i - iSignPosition - 1] = key[i];
		}
	}

	if (iSignPosition == -1 || iSignPosition == 0 || iSignPosition == keySize - 1)
	{
		return false;
	}

	tableName[iSignPosition] = 0;
	id = atoll(idStr);

	return true;
}

bool CRedisBase::IsKeyExists(const char* key)
{
	if (!key)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "EXISTS %s", key);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

// redis 中的key是否存在
bool CRedisBase::IsKeyExists(const char* mainKey, int mainID, int assID)
{
	if (!mainKey)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "EXISTS %s|%d,%d", mainKey, mainID, assID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

// redis 中的key是否存在
bool CRedisBase::IsKeyExists(const char* mainKey, int mainID)
{
	if (!mainKey)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "EXISTS %s|%d", mainKey, mainID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

// redis 中的key是否存在
bool CRedisBase::IsKeyExists(const char* mainKey, const char* assKey)
{
	if (!mainKey || !assKey)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "EXISTS %s|%s", mainKey, assKey);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisBase::DelKey(const char* key)
{
	if (!key)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "DEL %s", key);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

int CRedisBase::GetKeyType(const char* key)
{
	if (!key)
	{
		return REDIS_KEY_TYPE_NONE;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "TYPE %s", key);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int keyType = REDIS_KEY_TYPE_NONE;

	if (pReply->str && !strcmp(pReply->str, "hash"))
	{
		keyType = REDIS_KEY_TYPE_HASH;
	}

	freeReplyObject(pReply);

	return keyType;
}

bool CRedisBase::SremMember(const char* key, const char* member)
{
	if (!key || !member)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "SREM %s %s", key, member);
	if (!pReply)
	{
		return false;
	}

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		if (pReply->integer == 1)
		{
			ret = true;
		}
	}

	freeReplyObject(pReply);
	return ret;
}

// 删除集合（有序集合）元素
bool CRedisBase::ZremMember(const char* key, const std::vector<long long>& memberVec)
{
	int size = memberVec.size();
	if (size > 0)
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

		int iCount_ = (size - 1) / 150;
		for (int index_ = 0; index_ <= iCount_; index_++)
		{
			sprintf(redisCmd, "ZREM %s ", key);

			for (int i = index_ * 150; i < size && i < (index_ + 1) * 150; i++)
			{
				sprintf(redisCmd + strlen(redisCmd), "%lld ", memberVec[i]);
			}

			redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
			REDIS_CHECKF(pReply, redisCmd);

			freeReplyObject(pReply);
		}
	}

	return true;
}

// 删除集合（有序集合）元素(int类型)
bool CRedisBase::ZremMember(const char* key, const std::vector<int>& memberVec)
{
	int size = memberVec.size();
	if (size > 0)
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

		int iCount_ = (size - 1) / 150;
		for (int index_ = 0; index_ <= iCount_; index_++)
		{
			sprintf(redisCmd, "ZREM %s ", key);

			for (int i = index_ * 150; i < size && i < (index_ + 1) * 150; i++)
			{
				sprintf(redisCmd + strlen(redisCmd), "%d ", memberVec[i]);
			}

			redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
			REDIS_CHECKF(pReply, redisCmd);

			freeReplyObject(pReply);
		}
	}

	return true;
}

// 获取有序集合的大小
int CRedisBase::GetZSetSize(const char* key)
{
	if (!key)
	{
		return 0;
	}

	int iSize = 0;
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "ZCARD %s", key);
	if (!pReply)
	{
		return iSize;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		iSize = (int)pReply->integer;
	}

	freeReplyObject(pReply);

	return iSize;
}

bool CRedisBase::SetExpire(const char* key, int secs)
{
	if (!key)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "EXPIRE %s %d", key, secs);

	INFO_LOG("SetExpire cmd=%s", redisCmd);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = pReply->integer == 1 ? true : false;
	}

	freeReplyObject(pReply);

	return ret;
}

int CRedisBase::GetExtendMode(const char* key)
{
	if (!key)
	{
		return -1;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s extendMode", key);
	if (!pReply)
	{
		return -1;
	}

	int ret = -1;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisBase::SetExtendMode(const char* key, int mode)
{
	if (!key)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HSET %s extendMode %d", key, mode);
	if (!pReply)
	{
		return false;
	}

	bool ret = false;
	if (pReply->type != REDIS_REPLY_ERROR && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

void CRedisBase::CheckConnection(const RedisConfig& redisConfig)
{
	if (!m_pContext)
	{
		return;
	}

	bool needReconnect = true;
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "PING");
	if (pReply)
	{
		if (pReply->type == REDIS_REPLY_STATUS && !strcmp(pReply->str, "PONG"))
		{
			needReconnect = false;
		}

		freeReplyObject(pReply);
	}

	if (needReconnect)
	{
		ERROR_LOG("redis连接关闭，正在重新连接：type=%d,ip=%s,port=%d", redisConfig.redisTypeID, redisConfig.ip, redisConfig.port);
		ReConnect(redisConfig);
	}
}

bool CRedisBase::ReConnect(const RedisConfig& redisConfig)
{
	// 释放掉旧的
	redisFree(m_pContext);

	timeval tv = { 3, 0 };

	m_pContext = redisConnectWithTimeout(redisConfig.ip, redisConfig.port, tv);
	if (!m_pContext)
	{
		ERROR_LOG("重连redis失败:type=%d,ip=%s,port=%d", redisConfig.redisTypeID, redisConfig.ip, redisConfig.port);
		return false;
	}

	if (m_pContext->err != 0)
	{
		ERROR_LOG("ReConnect err err=%d info=%s", m_pContext->err, m_pContext->errstr);
		return false;
	}

	INFO_LOG("=== 重连redis-server:type=%d,ip=%s,port=%d ===", redisConfig.redisTypeID, redisConfig.ip, redisConfig.port);

	return true;
}

bool CRedisBase::Auth(const char* passwd)
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "AUTH %s", passwd);
	if (!pReply)
	{
		return false;
	}

	bool ret = false;
	if (pReply->type == REDIS_REPLY_STATUS)
	{
		if (!strcmp(pReply->str, "OK"))
		{
			ret = true;
		}
	}

	freeReplyObject(pReply);
	return ret;
}

bool CRedisBase::hmset(const char* table, int id, std::unordered_map<std::string, std::string>& fieldInfoUMap, int mode /*= REDIS_EXTEND_MODE_DEFAULT*/, const char* updateSet/* = NULL*/)
{
	char strID[14] = "";
	sprintf(strID, "%d", id);
	return hmset(table, strID, fieldInfoUMap, mode, updateSet);
}

bool CRedisBase::hmset(const char* table, long long id, std::unordered_map<std::string, std::string>& fieldInfoUMap, int mode /*= REDIS_EXTEND_MODE_DEFAULT*/, const char* updateSet/* = NULL*/)
{
	char strID[36] = "";
	sprintf(strID, "%lld", id);
	return hmset(table, strID, fieldInfoUMap, mode, updateSet);
}

bool CRedisBase::hmset(const char* table, const char* id, std::unordered_map<std::string, std::string>& fieldInfoUMap, int mode /*= REDIS_EXTEND_MODE_DEFAULT*/, const char* updateSet/* = NULL*/)
{
	if (fieldInfoUMap.size() >= MAX_REDIS_CMD_WORD_COUNT / 2)
	{
		ERROR_LOG("too much elements");
		return false;
	}

	const char* argv[MAX_REDIS_CMD_WORD_COUNT] = { NULL };
	size_t lens[MAX_REDIS_CMD_WORD_COUNT] = { 0 };
	char pMode[4] = "";

	int count = 0;
	std::string key = MakeKey(table, id);

	argv[count++] = "HMSET";
	argv[count++] = key.c_str();

	for (auto iter = fieldInfoUMap.begin(); iter != fieldInfoUMap.end(); ++iter)
	{
		argv[count++] = (iter->first).c_str();
		argv[count++] = (iter->second).c_str();
	}

	if (mode != REDIS_EXTEND_MODE_DEFAULT)
	{
		redisReply* pReply = NULL;

		// 先保存到集合
		pReply = (redisReply*)redisCommand(m_pContext, "SADD %s %s", updateSet == NULL ? CACHE_UPDATE_SET : updateSet, key.c_str());
		if (!pReply)
		{
			ERROR_LOG("redisCommand failed");
			return false;
		}
		freeReplyObject(pReply);

		const char* field = "extendMode";
		sprintf(pMode, "%d", mode);
		argv[count++] = field;
		argv[count++] = pMode;
	}

	for (int i = 0; i < count; i++)
	{
		lens[i] = strlen(argv[i]);
	}

	redisReply* pReply = (redisReply*)redisCommandArgv(m_pContext, count, argv, lens);
	if (!pReply)
	{
		ERROR_LOG("redisCommandArgv failed");
		return false;
	}

	if (pReply->type == REDIS_REPLY_ERROR)
	{
		ERROR_LOG("redisCommandArgv failed errstr=%s", pReply->str);
		freeReplyObject(pReply);
		return false;
	}

	bool ret = false;
	if (!strcmp(pReply->str, "OK"))
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

void CRedisBase::TestRedis(const char* name)
{
	redisReply* pReply = NULL;
	const int expireTime = 100;		// 失效时间(ms)
	int lockCount = 0;

	pReply = (redisReply*)redisCommand(m_pContext, "SET %s 0 PX %d NX", name, expireTime);
	freeReplyObject(pReply);

	while (true)
	{
		lockCount++;

		pReply = (redisReply*)redisCommand(m_pContext, "SET %s 0 PX %d NX", name, expireTime);
		if (!pReply)
		{
			ERROR_LOG("redisCommand return NULL");
			return;
		}

		if (pReply->type == REDIS_REPLY_STATUS)
		{
			if (!strcmp(pReply->str, "OK"))
			{
				// 设置成功了
				freeReplyObject(pReply);
				break;
			}
		}

		freeReplyObject(pReply);
	}

	m_sleepTime = expireTime * 10 / lockCount;
	if (m_sleepTime <= 0)
	{
		m_sleepTime = 1;
	}
	else if (m_sleepTime > 10)
	{
		m_sleepTime = 10;
	}

	INFO_LOG("%s 写数据 %d", name, lockCount);
}

// 获取某个键值索引
long long CRedisBase::GetKeyIndex(const char* key)
{
	long long llIndex = 1;

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "GET %s", key);
	if (!pReply)
	{
		return llIndex;
	}

	if (pReply->str == NULL || pReply->len == 0)
	{
		freeReplyObject(pReply);
		return llIndex;
	}

	llIndex = atoll(pReply->str);

	freeReplyObject(pReply);

	return llIndex;
}

// 设置某个键值索引
bool CRedisBase::SetKeyIndex(const char* key, long long llIndex)
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "SET %s %lld", key, llIndex);
	if (!pReply)
	{
		return false;
	}

	freeReplyObject(pReply);

	return true;
}

// 获取自增字符串
long long CRedisBase::GetIncrKeyIndex(const char* key)
{
	if (key == NULL)
	{
		return 0;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "INCR %s", key);
	if (!pReply)
	{
		return 0;
	}

	long long ret = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = pReply->integer;
	}
	freeReplyObject(pReply);

	return ret;
}

// 数据库
void CRedisBase::SetDBManage(CDataBaseManage* pDBManage)
{
	if (pDBManage)
	{
		m_pDBManage = pDBManage;
	}
}

//////////////////////////////////////////redis lock///////////////////////////////////////////////////////////////////
CRedisLock::CRedisLock(redisContext* pContext, const char* key, int sleepTime /*= 0*/, bool bAutoLock/* = true*/)
{
	m_pContext = NULL;
	memset(m_key, 0, sizeof(m_key));
	m_sleepTime = sleepTime;

	if (pContext && key)
	{
		m_pContext = pContext;
		sprintf(m_key, "%s.lock", key);
		if (bAutoLock)
		{
			Lock();
		}
	}
}

CRedisLock::~CRedisLock()
{
	Unlock();
}

void CRedisLock::Lock()
{
	if (!m_pContext || m_key[0] == '\0')
	{
		return;
	}

	redisReply* pReply = NULL;
	const int expireTime = 1000;		// 失效时间(ms)
	int lockCount = 0;

	while (true)
	{
		lockCount++;

		pReply = (redisReply*)redisCommand(m_pContext, "SET %s 0 PX %d NX", m_key, expireTime);
		if (!pReply)
		{
			ERROR_LOG("redisCommand return NULL");
			return;
		}

		if (pReply->type == REDIS_REPLY_STATUS)
		{
			if (!strcmp(pReply->str, "OK"))
			{
				// 设置成功了
				freeReplyObject(pReply);
				break;
			}
		}

		if (pReply->type == REDIS_REPLY_ERROR)
		{
			ERROR_LOG("##########  设置分布式锁失败，可能会导致数据同步问题 ERROR:【%s】  ###########", pReply->str);
			freeReplyObject(pReply);
			break;
		}

		freeReplyObject(pReply);

		if (m_sleepTime > 0 && m_sleepTime <= 100)
		{
			usleep(m_sleepTime * 1000);
		}
	}

	if (lockCount >= 2)
	{
		INFO_LOG("RedisLock超时 lockCount=%d m_sleepTime=%d key=%s ", lockCount, m_sleepTime, m_key);
	}
}

void CRedisLock::Unlock()
{
	if (m_key[0] == '\0')
	{
		return;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "DEL %s", m_key);
	if (!pReply)
	{
		ERROR_LOG("redisCommand return NULL");
		return;
	}

	freeReplyObject(pReply);

	m_key[0] = '\0';
}
