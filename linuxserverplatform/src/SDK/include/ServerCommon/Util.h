#pragma once

enum TimeFormat
{
	TIME_YYYYMMDD = 0,		// 20170802
	TIME_YYYYMMDDHHmm,		// 201708021200
};

struct Util_KeyValueStruct_
{
	int key_;
	long long value_;
	Util_KeyValueStruct_()
	{
		memset(this, 0, sizeof(Util_KeyValueStruct_));
	}
};

// 工具类
class CUtil
{
public:
	CUtil();
	~CUtil();

public:
	// 检查字符串
	static bool CheckString(const char* src);
	// 将#和%号替换成？
	static bool CheckString(char* src, int size);
	// 分割字符串
	static void SplitString(std::string str, std::string pattern, std::vector<std::string> &vecSplitData);
	static std::string &TrimString(std::string &s);
	// 获取uuid
	static void GetUuid(char* out, int len);
	// MD5加密
	static void MD5(std::string &str);
	// 获取当前时间
	static long long GetTime(int format);
	// 从时间戳中获取日期
	static int GetDateFromTimeStamp(time_t time);
	// 从时间戳中获取weekday (0-6)
	static int GetWeekdayFromTimeStamp(time_t time);
	// 从时间戳中获取小时 (0-23)
	static int GetHourTimeStamp(time_t time);
	// 解析json数据
	static std::string ParseJsonValue(const std::string& src, const char* key);
	// 是否包含敏感词
	static bool IsContainDirtyWord(const std::string& str);
	// 如果文件夹不存在则创建文件夹
	static void MkdirIfNotExists(const char* dir);
	// 字符串转换
	static std::string Tostring(BYTE value);
	static std::string Tostring(int value);
	static std::string Tostring(long value);
	static std::string Tostring(size_t value);
	static std::string Tostring(long long value);
	static std::string Tostring(double value);
	// 替换字符串
	static void ReplaceStr(char str[], int count, char src, char dst);
	// 字符串解析函数
	static bool GetKeyAndValue(const char * pStr, int &iKey, long long &llValue);
	static void ArrayToString(long long * pArray, int iArrayCount, char * pStr);
	static void StringToArray(char * pStr, long long * pArray, int &iArrayCount);
	static void KYArrayToString(Util_KeyValueStruct_ * pKYArray, int iArrayCount, char * pStr);
	static void StringToKYArray(char * pStr, Util_KeyValueStruct_ * pKYArray, int &iArrayCount);
	// 获取一个随机数。范围0-65535
	static unsigned int GetRandNum();
	// 获取[A,B)随机数,min<= 随机数 < iMax
	static int GetRandRange(int iMin, int iMax);
	// 转义字符串
	static void TransString(char * pStr, int iCount, int iMaxSignCount, bool bUTF = false);
	// 字符串哈希函数
	static unsigned int BKDRHash(const char *str);
	static unsigned int APHash(const char *str);
	static unsigned int DJBHash(const char *str);
	static bool GetScoreFromUserInfoList(int userID, char* userInfoList, long long &llScore);
	// 校验头部
	static bool CheckCode(UINT uReserve);
	// 根据经纬度计算距离（单位m）
	static double GetDistanceVer(const char * lat1, const char *  lng1, const char *  lat2, const char *  lng2);
};