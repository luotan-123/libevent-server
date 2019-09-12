#pragma once

#include <map>
#include <string>

using namespace std;

#define CONFIGLEN           256 

enum INI_RES
{
	INI_SUCCESS,            //成功
	INI_ERROR,              //普通错误
	INI_OPENFILE_ERROR,     //打开文件失败
	INI_NO_ATTR             //无对应的键值
};

//构造函数 mode 字符串说明
//r	以只读方式打开文件，该文件必须存在。
//r + 以读 / 写方式打开文件，该文件必须存在。
//rb + 以读 / 写方式打开一个二进制文件，只允许读 / 写数据。
//rt + 以读 / 写方式打开一个文本文件，允许读和写。
//w	打开只写文件，若文件存在则文件长度清为零，即该文件内容会消失；若文件不存在则创建该文件。
//w + 打开可读 / 写文件，若文件存在则文件长度清为零，即该文件内容会消失；若文件不存在则创建该文件。
//a	以附加的方式打开只写文件。若文件不存在，则会创建该文件；如果文件存在，则写入的数据会被加到文件尾后，即文件原先的内容会被保留（EOF 符保留）。
//a + 以附加方式打开可读 / 写的文件。若文件不存在，则会创建该文件，如果文件存在，则写入的数据会被加到文件尾后，即文件原先的内容会被保留（EOF符不保留）。

//子键索引 子键值 
typedef map<std::string, std::string> KEYMAP;
//主键索引 主键值  
typedef map<std::string, KEYMAP> MAINKEYMAP;

// config 文件的基本操作类
class CINIFile
{
public:
	// 构造函数
	CINIFile(string fileName, string mode = "r+");
	// 析够函数
	virtual ~CINIFile();
public:
	// 判断文件是否存在
	static bool IsFileExist(string fileName);
	// 拷贝文件
	static void CopyFileTo(string destFileName, string srcFileName);
	// 获取当前目录
	static string GetAppPath();
	// 打开config 文件
	INI_RES OpenFile(const char* pathName, const char* mode);
	// 关闭config 文件
	INI_RES CloseFile();
	// 获取整形的键值
	int  GetKeyVal(string secName, string keyName, int default_);
	// 获取无符号整形的键值
	UINT GetKeyVal(string secName, string keyName, UINT default_);
	// 获取长整形的键值
	long long GetKeyVal(string secName, string keyName, long long default_);
	// 获取double的键值
	double  GetKeyVal(string secName, string keyName, double default_);
	// 获取键值的字符串
	string GetKeyVal(string secName, string keyName, string default_);
	// 设置键值的字符串
	void SetKeyValString(string secName, string keyName, string str);

protected:
	// 读写config文件
	INI_RES GetKey(string secName, string keyName, string &value);
	void SetKey(string secName, string keyName, string value);
protected:
	// 被打开的文件局柄
	FILE* m_fp;
	string  m_szKey;
	MAINKEYMAP m_Map;
	bool  m_bModify;
};

