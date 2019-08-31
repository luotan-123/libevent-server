#pragma once

#include <map>
#include <algorithm>
#include <string>
#include <direct.h>
#include <Windows.h>
#include <tchar.h>

#pragma warning(disable:4996)

using namespace std;

#define MAX_STRING_SIZE 128

class CINIFile
{
public:
	CINIFile(string FileName, int maxsize = MAX_STRING_SIZE)
	{
		lpFileName = FileName;
		mMaxSize = maxsize;
	}

	~CINIFile()
	{
		//lpFileName.Empty();
	}

public:
	static __int64 my_atoi(const char* str)
	{
		__int64 result = 0;
		int signal = 1;
		/* 默认为正数 */
		if ((*str >= '0' && *str <= '9') || *str == '-' || *str == '+')
		{
			if (*str == '-' || *str == '+')
			{
				if (*str == '-')
					signal = -1; /* 输入负数 */
				str++;
			}
		}
		else return 0;/* 开始转换 */
		while (*str >= '0' && *str <= '9')
			result = result * 10 + (*str++ - '0');
		return signal * result;
	}

	static bool IsFileExist(string fileName)
	{
		WIN32_FIND_DATA  d;
		HANDLE  hd = FindFirstFile(fileName.c_str(), &d);
		bool bRet = (hd != INVALID_HANDLE_VALUE);
		FindClose(hd);
		return bRet;
	}

	static void CopyFileTo(string destFileName, string srcFileName)
	{
		bool bb = IsFileExist(srcFileName);
		if (!bb)return;
		CopyFile(srcFileName.c_str(), destFileName.c_str(), FALSE);
	}

	static string GetWinSysPath()
	{
		char lpBuffer[MAX_STRING_SIZE];
		GetSystemDirectory(lpBuffer, MAX_STRING_SIZE);
		string ss = lpBuffer;
		ss += "\\";
		return ss;
	}

	static string GetAppPath(bool bFource = true)
	{
		TCHAR szModuleName[MAX_PATH] = "";
		DWORD dwLength = GetModuleFileName(GetModuleHandle(NULL), szModuleName, sizeof(szModuleName));
		string filepath = szModuleName;
		string path;
		int nSlash = filepath.find_last_of(_T('/'));
		if (nSlash == -1)
		{
			nSlash = filepath.find_last_of(_T('\\'));
		}

		if (nSlash != -1 && filepath.length() > 1)
		{
			path = filepath.substr(0, nSlash + 1);
			SetCurrentDirectory(path.c_str());
			return path;
		}
		else
		{
			char lpBuffer[MAX_STRING_SIZE] = "";
			GetCurrentDirectory(MAX_STRING_SIZE, lpBuffer);
			string ss = lpBuffer;
			ss += "\\";
			return ss;
		}
	}

	int GetKeyVal(string secName, string keyName, int lpDefault)
	{
		return (int)GetPrivateProfileInt(
			secName.c_str(),/// points to section name
			keyName.c_str(),/// points to key name
			lpDefault,       /// return value if key name not found
			lpFileName.c_str()///LPCTSTR lpFileName  /// initialization file name
		);
	}

	UINT GetKeyVal(string secName, string keyName, UINT lpDefault)
	{
		return (UINT)GetPrivateProfileInt(
			secName.c_str(),/// points to section name
			keyName.c_str(),/// points to key name
			lpDefault,       /// return value if key name not found
			lpFileName.c_str()///LPCTSTR lpFileName  /// initialization file name
		);
	}

	__int64 GetKeyVal(string secName, string keyName, __int64 lpDefault)
	{
		char str[255] = "";
		sprintf_s(str, sizeof(str), "%I64d", lpDefault);
		string __int64Str = GetKeyVal(secName, keyName, str);
		return my_atoi(__int64Str.c_str());
	}

	string GetKeyVal(string secName, string keyName, LPCTSTR lpDefault)
	{
		char* re = new char[mMaxSize + 1];
		ZeroMemory(re, mMaxSize);
		GetPrivateProfileString(
			secName.c_str(),/// points to section name
			keyName.c_str(),/// points to key name
			lpDefault,/// points to default string
			re,/// points to destination buffer
			mMaxSize,/// size of destination buffer
			lpFileName.c_str() /// points to initialization filename
		);
		string ss = re;
		delete[]re;
		return ss;
	}

	void SetKeyValString(string secName, string keyName, string Val)
	{
		WritePrivateProfileString(
			secName.c_str(),/// pointer to section name
			keyName.c_str(),/// pointer to key name
			Val.c_str(),/// pointer to string to add
			lpFileName.c_str()/// pointer to initialization filename
		);
	}

	void SetINIFileName(string fileName)
	{
		lpFileName = fileName;
	}

public:
	string lpFileName;
	DWORD mMaxSize;
};