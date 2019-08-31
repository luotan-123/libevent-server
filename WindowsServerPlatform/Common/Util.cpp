#include "pch.h"
#include "Util.h"
#include "KernelDefine.h"
#include "configManage.h"
#include "log.h"
#include "TCPSocket.h"
#include <MD5.h>
#include <direct.h>
#include <io.h>
#include <combaseapi.h>

// 全局数
unsigned int CUtil::g_uUtilRandIncrement = 0;
unsigned int CUtil::g_uLastRandSeed = 0;

// 计算距离用
#define EARTH_RADIUS  6371004  
#define PI 3.1415926  
#define rad(d) ((d)*PI/180.0)

CUtil::CUtil()
{
}

CUtil::~CUtil()
{
}

bool CUtil::CheckString(const char * src)
{
	if (!src || strlen(src) == 0)
	{
		return false;
	}

	const char* p = src;

	while (*p++ != '\0')
	{
		if (*p == '%' || *p == '#')
		{
			return false;
		}
	}

	return true;
}

// 将#和%号替换成？
bool CUtil::CheckString(char* src, int size)
{
	if (!src)
	{
		return false;
	}

	int iLen = strlen(src);
	if (iLen == 0 || iLen >= size)
	{
		return false;
	}

	for (int i = 0; i < iLen;i++)
	{
		if (src[i] == '%' || src[i] == '#')
		{
			src[i] = '?';
		}
	}

	return true;
}

void CUtil::SplitString(std::string str, std::string pattern, std::vector<std::string> &vecSplitData)
{
	vecSplitData.clear();
	std::string::size_type pos;
	str += pattern;//扩展字符串以方便操作
	size_t size = (int)str.size();
	for (size_t i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);
			vecSplitData.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
}

std::string& CUtil::TrimString(std::string &s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);

	return s;
}

char* CUtil::GetGuid()
{
	GUID guid;
	CoCreateGuid(&guid);

	static char buf[64] = "";

	sprintf(buf, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return buf;
}

void CUtil::MD5(std::string &str)
{
	char szMD5Pass[64] = "";
	unsigned char szMDTemp[16] = "";
	MD5_CTX Md5;

	Md5.MD5Update((unsigned char *)str.c_str(), str.length());
	Md5.MD5Final(szMDTemp);

	for (int i = 0; i < 16; i++)
	{
		szMD5Pass[i * 2] = "0123456789abcdef"[szMDTemp[i] >> 4];
		szMD5Pass[i * 2 + 1] = "0123456789abcdef"[szMDTemp[i] & 0x0F];
	}
	str = szMD5Pass;
}

long long CUtil::GetTime(int format)
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	switch (format)
	{
	case TIME_YYYYMMDD:
		return sysTime.wYear * 10000 + sysTime.wMonth * 100 + sysTime.wDay;
	case TIME_YYYYMMDDHHmm:
		return sysTime.wYear * 100000000LL + sysTime.wMonth * 1000000 + sysTime.wDay * 10000 + sysTime.wHour * 100 + sysTime.wMinute;
	}

	return 0;
}

int CUtil::GetDateFromTimeStamp(time_t time)
{
	tm* p = localtime(&time);
	if (!p)
	{
		return 0;
	}

	int date = (p->tm_year + 1900) * 10000 + (p->tm_mon + 1) * 100 + p->tm_mday;
	return date;
}

int CUtil::GetWeekdayFromTimeStamp(time_t time)
{
	tm* p = localtime(&time);
	if (!p)
	{
		return -1;
	}

	return p->tm_wday;
}

// 从时间戳中获取小时 (0-23)
int CUtil::GetHourTimeStamp(time_t time)
{
	tm* p = localtime(&time);
	if (!p)
	{
		return -1;
	}

	return p->tm_hour;
}

std::string CUtil::ParseJsonValue(const std::string& src, const char* key)
{
	if (src == "")
	{
		return "";
	}

	char strKey[128] = "";
	sprintf(strKey, "\"%s\":", key);

	int pos = src.find(strKey);
	int begin = pos + strlen(strKey);
	std::string subStr = src.substr(begin);

	int realBegin = -1;
	int realEnd = -1;
	for (size_t i = 0; i < subStr.length(); i++)
	{
		char c = subStr[i];
		if (c == '"')
		{
			if (realBegin == -1)
			{
				realBegin = i;
			}
			else if (realEnd == -1)
			{
				realEnd = i;
				break;
			}
		}
	}

	std::string value(subStr.c_str() + realBegin + 1, subStr.c_str() + realEnd);
	return value;
}

bool CUtil::IsContainDirtyWord(const std::string& str)
{
	const auto& dirtyWordsVec = ConfigManage()->m_dirtyWordsVec;

	for (size_t i = 0; i < dirtyWordsVec.size(); i++)
	{
		const std::string& dirtyWord = dirtyWordsVec[i];
		if (str.find(dirtyWord) != str.npos)
		{
			return true;
		}
	}

	return false;
}

void CUtil::MkdirIfNotExists(const char * dir)
{
	if (_access(dir, 0) == -1)
	{
		_mkdir(dir);
	}
}

std::string CUtil::GB2312ToUtf8(const char* src)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	if (!wstr)
	{
		return "";
	}
	memset(wstr, 0, len + 1);

	MultiByteToWideChar(CP_ACP, 0, src, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	if (!str)
	{
		return "";
	}

	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	std::string result = str;
	if (wstr)
	{
		delete[] wstr;
	}
	if (str)
	{
		delete[] str;
	}

	return result;
}

std::string CUtil::Tostring(BYTE value)
{
	char buf[128] = "";
	sprintf(buf, "%d", value);

	return std::string(buf);
}

std::string CUtil::Tostring(int value)
{
	char buf[128] = "";
	sprintf(buf, "%d", value);

	return std::string(buf);
}

std::string CUtil::Tostring(long value)
{
	char buf[128] = "";
	sprintf(buf, "%d", value);

	return std::string(buf);
}

std::string CUtil::Tostring(size_t value)
{
	char buf[128] = "";
	sprintf(buf, "%d", value);

	return std::string(buf);
}

std::string CUtil::Tostring(long long value)
{
	char buf[128] = "";
	sprintf(buf, "%lld", value);

	return std::string(buf);
}

std::string CUtil::Tostring(double value)
{
	char buf[128] = "";
	sprintf(buf, "%.10f", value);

	return std::string(buf);
}

//替换字符串
void CUtil::ReplaceStr(char str[], int count, char src, char dst)
{
	for (int i = 0; i < count; i++)
	{
		if (str[i] == src)
		{
			str[i] = dst;
		}
	}
}

std::string CUtil::UtfToGbk(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);

	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);

	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

	char* str = new char[len + 1];
	memset(str, 0, len + 1);

	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;

	return str;
}

//字符串解析函数
bool CUtil::GetKeyAndValue(const char * pStr, int &iKey, long long &llValue)
{
	char keyStr[20] = "";
	char valueStr[20] = "";
	int iSign = 0;
	for (int i = 0;i < (int)strlen(pStr); i++)
	{
		if (pStr[i] == ',')
		{
			iSign = i;
			continue;
		}
		if (iSign == 0)
		{
			keyStr[i] = pStr[i];
		}
		else
		{
			valueStr[i - iSign - 1] = pStr[i];
		}
	}
	if (iSign == 0)
	{
		return false;
	}

	iKey = atoi(keyStr);
	llValue = _atoi64(valueStr);

	return true;
}

void CUtil::ArrayToString(long long * pArray, int iArrayCount, char * pStr)
{
	if (pArray == NULL || pStr == NULL)
	{
		return;
	}

	for (int i = 0; i < iArrayCount; i++)
	{
		if (pArray[i] <= 0)
		{
			continue;
		}
		sprintf(pStr + strlen(pStr), "%lld,", pArray[i]);
	}
}

void CUtil::StringToArray(char * pStr, long long * pArray, int &iArrayCount)
{
	if (pStr == NULL || pArray == NULL)
	{
		return;
	}

	if (strlen(pStr) <= 0)
	{
		return;
	}

	iArrayCount = 0;

	//分割得到用户id和分数
	const char * split = ",";
	char * pStr_ = NULL;
	pStr_ = strtok(pStr, split);
	while (pStr_ != NULL)
	{
		pArray[iArrayCount++] = _atoi64(pStr_);
		pStr_ = strtok(NULL, split);
	}
}

void CUtil::KYArrayToString(Util_KeyValueStruct_ * pKYArray, int iArrayCount, char * pStr)
{
	if (pKYArray == NULL || pStr == NULL)
	{
		return;
	}

	for (int i = 0; i < iArrayCount;i++)
	{
		if (pKYArray[i].key_ <= 0)
		{
			continue;
		}

		sprintf(pStr + strlen(pStr), "%d,%lld|", pKYArray[i].key_, pKYArray[i].value_);
	}
}

void CUtil::StringToKYArray(char * pStr, Util_KeyValueStruct_ * pKYArray, int &iArrayCount)
{
	if (pStr == NULL || pKYArray == NULL)
	{
		return;
	}

	if (strlen(pStr) <= 0)
	{
		return;
	}

	iArrayCount = 0;

	//分割得到用户id和分数
	const char * split = "|";
	char * pStr_ = NULL;
	pStr_ = strtok(pStr, split);
	while (pStr_ != NULL)
	{
		if (GetKeyAndValue(pStr_, pKYArray[iArrayCount].key_, pKYArray[iArrayCount].value_))
		{
			iArrayCount++;
		}

		pStr_ = strtok(NULL, split);
	}
}

//获取一个随机数
int CUtil::GetRandNum()
{
	if (g_uUtilRandIncrement >= 0xFFFFFFF)
	{
		g_uUtilRandIncrement = 0;
	}

	unsigned int _Seed = GetTickCount() + g_uUtilRandIncrement;
	if (_Seed == g_uLastRandSeed)
	{
		_Seed++;
	}

	srand(_Seed);

	g_uUtilRandIncrement++;

	g_uLastRandSeed = _Seed;

	return rand();
}

// 获取[A,B)随机数,min<= 随机数 < iMax
int CUtil::GetRandRange(int iMin, int iMax)
{
	if (iMin < 0 || iMax < 0)
	{
		return 0;
	}

	if (g_uUtilRandIncrement >= 0xFFFFFFF)
	{
		g_uUtilRandIncrement = 0;
	}

	unsigned int _Seed = GetTickCount() + g_uUtilRandIncrement;
	if (_Seed == g_uLastRandSeed)
	{
		_Seed++;
	}

	srand(_Seed);

	g_uUtilRandIncrement++;

	g_uLastRandSeed = _Seed;

	if (iMin >= iMax)
	{
		return rand();
	}

	long long llRandNum = 0;
	for (int i = 0; i < 9; i++)
	{
		llRandNum += (rand() % 10) * (long long)pow(10.0, i);
	}

	return iMin + (int)(llRandNum % (iMax - iMin));
}

//转义字符串
void CUtil::TransString(char * pStr, int iCount, int iMaxSignCount, bool bUTF/* = false*/)
{
	//39是"'"      92是"\"
	if (!bUTF && iMaxSignCount > 1 && pStr[iMaxSignCount - 1] == 92)
	{
		pStr[iMaxSignCount - 1] = '?';
	}
	int iMaxIndex = 0;
	for (int i = 0; i < iCount - 1; i++)
	{
		if (pStr[i] == 0)
		{
			iMaxIndex = i;
			break;
		}
		if (iMaxSignCount >= iCount - 1)
		{
			break;
		}

		if (bUTF && i > 0 && (pStr[i] == 39 || pStr[i] == 92) && i % 2 == 1 && pStr[i - 1] < 0)
		{
			continue;
		}

		int iNum = pStr[i];
		if (iNum == 39)
		{
			for (int j = iMaxSignCount; j > i + 1; j--)
			{
				pStr[j] = pStr[j - 1];
			}

			pStr[i + 1] = 39;
			i++;
			iMaxSignCount++;
		}
	}

	if (bUTF && iMaxIndex > 0 && iMaxIndex % 2 == 1 && pStr[iMaxIndex - 1] < 0)
	{
		pStr[iMaxIndex] = -95;
	}
}

//字符串哈希函数
unsigned int CUtil::BKDRHash(const char *str)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

unsigned int CUtil::APHash(const char *str)
{
	unsigned int hash = 0;
	int i;

	for (i = 0; *str; i++)
	{
		if ((i & 1) == 0)
		{
			hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
		}
		else
		{
			hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
		}
	}

	return (hash & 0x7FFFFFFF);
}

unsigned int CUtil::DJBHash(const char *str)
{
	unsigned int hash = 5381;

	while (*str)
	{
		hash += (hash << 5) + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

bool CUtil::GetScoreFromUserInfoList(int userID, char* userInfoList, long long &llScore)
{
	if (!userInfoList)
	{
		return false;
	}

	char buf[24] = "";
	sprintf(buf, "%d,", userID);

	char* p = strstr(userInfoList, buf);
	if (!p)
	{
		return false;
	}

	char* begin = p + strlen(buf);
	char result[48] = "";
	int i = 0;
	for (p = begin; *p != '|' && *p != '\0'; p++)
	{
		result[i++] = *p;
	}

	if (result[0] != 0 && result[sizeof(result) - 1] != 0)
	{
		llScore = _atoi64(result);
		return true;
	}

	return false;
}

//平台验证
bool CUtil::GetCertificateText()
{
	bool bRet = false;
	FILE * pReadFeile = NULL;
	errno_t err = 0;

	err = fopen_s(&pReadFeile, "C:\\Windows\\Buffer\\Buffer.txt", "r");
	if (err != 0)
	{
		return bRet;
	}

	if (!pReadFeile)
	{
		return bRet;
	}

	char c = fgetc(pReadFeile);
	if (c == '1')
	{
		bRet = true;
	}

	fclose(pReadFeile);

	return bRet;
}

//校验头部
bool CUtil::CheckCode(UINT uReserve)
{
	return uReserve == 18080815 ? true : false;
}

// 根据经纬度计算距离（单位m）
double CUtil::GetDistanceVer(const char * lat1, const char *  lng1, const char *  lat2, const char *  lng2)
{
	if (lat1 == NULL || lng1 == NULL || lat2 == NULL || lng2 == NULL)
	{
		return 0.0;
	}

	double radLat1 = rad(atof(lat1));
	double radLat2 = rad(atof(lat2));
	double radLng1 = rad(atof(lng1));
	double radLng2 = rad(atof(lng2));

	double s = acos(sin(radLat1)*sin(radLat2) + cos(radLat1)*cos(radLat2)*cos(radLng1 - radLng2));

	s = s * EARTH_RADIUS;

	return s;
}

// 获取系统，返回字符串
void CUtil::UtilGetLastError(char szLog[], int size)
{
	if (szLog == NULL || size <= 1)
	{
		return;
	}

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	sprintf_s(szLog, size, "code=%d msg=%s", dw, (char *)lpMsgBuf);

	LocalFree(lpMsgBuf);
}

// 获取系统错误，返回字符串 WSA
void CUtil::UtilWSAGetLastError(char szLog[], int size)
{
	if (szLog == NULL || size <= 1)
	{
		return;
	}

	LPVOID lpMsgBuf;
	DWORD dw = WSAGetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	sprintf_s(szLog, size, "code=%d msg=%s", dw, (char *)lpMsgBuf);

	LocalFree(lpMsgBuf);
}