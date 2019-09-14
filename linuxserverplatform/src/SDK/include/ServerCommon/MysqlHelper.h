#pragma once
#include <map>
#include <vector>
#include <string>
#include "mysql.h"

using namespace std;

/*********************
*@brief 数据库异常类
**********************/
struct MysqlHelper_Exception //: public TC_Exception
{
	MysqlHelper_Exception(const string& sBuffer) :errorInfo(sBuffer) {}; //: TC_Exception(sBuffer){};
	~MysqlHelper_Exception() throw() {};

	string errorInfo;
};

/***********************
* @brief 数据库配置接口
***********************/
struct DBConf
{
	string _host;//主机地址
	string _user; //用户名 
	string _password;//密码
	string _database; //数据库
	string _charset; //字符集
	int _port;//端口
	int _flag; //客户端标识

	/*****************
	* @brief 构造函数
	*****************/
	DBConf() :_port(0), _flag(0) {}

	/**********************************
	* @brief 读取数据库配置.
	* @param mpParam 存放数据库配置的map
	* dbhost: 主机地址
	* dbuser:用户名
	* dbpass:密码
	* dbname:数据库名称
	* dbport:端口
	**********************************/
	void loadFromMap(const map<string, string>& mpParam)
	{
		map<string, string> mpTmp = mpParam;

		_host = mpTmp["dbhost"];
		_user = mpTmp["dbuser"];
		_password = mpTmp["dbpass"];
		_database = mpTmp["dbname"];
		_charset = mpTmp["charset"];
		_port = atoi(mpTmp["dbport"].c_str());
		_flag = 0;

		if (mpTmp["dbport"] == "")
		{
			_port = 3306;
		}
	}
};

/**************************************************************
* @brief:MySQL数据库操作类
* @feature:非线程安全，通常一个线程一个CMysqlHelper对象；
* 对于insert/update可以有更好的函数封装，保证SQL注入；
* CMysqlHelper::DB_INT表示组装sql语句时，不加””和转义；
* CMysqlHelper::DB_STR表示组装sql语句时，加””并转义；
**************************************************************/
class CMysqlHelper
{
public:
	/**
	* @brief 构造函数
	*/
	CMysqlHelper();

	/**
	* @brief 构造函数.
	* @param: sHost:主机IP
	* @param sUser 用户
	* @param sPasswd 密码
	* @param sDatebase 数据库
	* @param port 端口
	* @param iUnixSocket socket
	* @param iFlag 客户端标识
	*/
	CMysqlHelper(const string& sHost, const string& sUser = "", const string& sPasswd = "", const string& sDatabase = "", const string& sCharSet = "", int port = 0, int iFlag = 0);

	/**
	* @brief 构造函数.
	* @param tcDBConf 数据库配置
	*/
	CMysqlHelper(const DBConf& tcDBConf);

	/**
	* @brief 析构函数.
	*/
	~CMysqlHelper();

	/**
	* @brief 初始化.
	*
	* @param sHost 主机IP
	* @param sUser 用户
	* @param sPasswd 密码
	* @param sDatebase 数据库
	* @param port 端口
	* @param iUnixSocket socket
	* @param iFlag 客户端标识
	* @return 无
	*/
	void init(const string& sHost, const string& sUser = "", const string& sPasswd = "", const string& sDatabase = "", const string& sCharSet = "", int port = 3306, int iFlag = 0);

	/**
	* @brief 初始化.
	*
	* @param tcDBConf 数据库配置
	*/
	void init(const DBConf& tcDBConf);

	/**
	* @brief 连接数据库.
	*
	* @throws MysqlHelper_Exception
	* @return 无
	*/
	void connect();

	/**
	* @brief 断开数据库连接.
	* @return 无
	*/
	void disconnect();

	/**
	* @brief 获取数据库变量.
	* @return 数据库变量
	*/
	string getVariables(const string& sName);

	/**
	* @brief 直接获取数据库指针.
	*
	* @return MYSQL* 数据库指针
	*/
	MYSQL* getMysql();

	/**
	* @brief 字符转义.
	*
	* @param sFrom 源字符串
	* @param sTo 输出字符串
	* @return 输出字符串
	*/
	string escapeString(const string& sFrom);

	/**
	* @brief 更新或者插入数据.
	*
	* @param sSql sql语句
	* @throws MysqlHelper_Exception
	* @return
	*/
	void execute(const string& sSql);

	/**
	* @brief 查询出来的mysql数据
	*/
	class MysqlData
	{
	public:
		/**
		* @brief 所有数据.
		*
		* @return vector<map<string,string>>&
		*/
		vector<map<string, string> >& data();

		/**
		* 数据的记录条数
		*
		* @return size_t
		*/
		size_t size();

		/**
		* @brief 获取某一条记录.
		*
		* @param i 要获取第几条记录
		* @return MysqlRecord类型的数据，可以根据字段获取相关信息，
		*/
		map<string, string>& operator[](size_t i);

	protected:
		vector<map<string, string> > _data;
	};

	/**
	* @brief Query Record.
	*
	* @param sSql sql语句
	* @throws MysqlHelper_Exception
	* @return MysqlData类型的数据，可以根据字段获取相关信息
	*/
	bool queryRecord(const string& sSql, MysqlData& data, bool bSetGBK = false);

	/**
	* @brief 执行sql语句
	*
	* @param sSql sql语句
	* @throws MysqlHelper_Exception
	* @return 成功返回true，失败返回false
	*/
	void sqlExec(const char* sql, bool bSetGBK = false);

	/**
	* @brief 定义字段类型，
	* DB_INT:数字类型
	* DB_STR:字符串类型
	*/
	enum FT
	{
		DB_INT,
		DB_STR,
	};

	/**
	* 数据记录
	*/
	typedef map<string, pair<FT, string> > RECORD_DATA;

	/**
	* @brief 更新记录.
	*
	* @param sTableName 表名
	* @param mpColumns 列名/值对
	* @param sCondition where子语句,例如:where A = B
	* @throws MysqlHelper_Exception
	* @return size_t 影响的行数
	*/
	size_t updateRecord(const string& sTableName, const map<string, pair<FT, string> >& mpColumns, const string& sCondition);

	/**
	* @brief 插入记录.
	*
	* @param sTableName 表名
	* @param mpColumns 列名/值对
	* @throws MysqlHelper_Exception
	* @return size_t 影响的行数
	*/
	size_t insertRecord(const string& sTableName, const map<string, pair<FT, string> >& mpColumns);

	/**
	* @brief 替换记录.
	*
	* @param sTableName 表名
	* @param mpColumns 列名/值对
	* @throws MysqlHelper_Exception
	* @return size_t 影响的行数
	*/
	size_t replaceRecord(const string& sTableName, const map<string, pair<FT, string> >& mpColumns);

	/**
	* @brief 删除记录.
	*
	* @param sTableName 表名
	* @param sCondition where子语句,例如:where A = B
	* @throws MysqlHelper_Exception
	* @return size_t 影响的行数
	*/
	size_t deleteRecord(const string& sTableName, const string& sCondition = "");

	/**
	* @brief 获取Table查询结果的数目.
	*
	* @param sTableName 用于查询的表名
	* @param sCondition where子语句,例如:where A = B
	* @throws MysqlHelper_Exception
	* @return size_t 查询的记录数目
	*/
	size_t getRecordCount(const string& sTableName, const string& sCondition = "");

	/**
	* @brief 获取Sql返回结果集的个数.
	*
	* @param sCondition where子语句,例如:where A = B
	* @throws MysqlHelper_Exception
	* @return 查询的记录数目
	*/
	size_t getSqlCount(const string& sCondition = "");

	/**
	* @brief 获取字段最大值.
	*
	* @param sTableName 用于查询的表名
	* @param sFieldName 用于查询的字段
	* @param sCondition where子语句,例如:where A = B
	* @throws MysqlHelper_Exception
	* @return 查询的记录数目
	*/
	int getMaxValue(const string& sTableName, const string& sFieldName, const string& sCondition = "");

	/**
	* @brief 获取auto_increment最后插入得ID.
	*
	* @return ID值
	*/
	long lastInsertID();

	/**
	* @brief 构造Insert-SQL语句.
	*
	* @param sTableName 表名
	* @param mpColumns 列名/值对
	* @return string insert-SQL语句
	*/
	string buildInsertSQL(const string& sTableName, const map<string, pair<FT, string> >& mpColumns);

	/**
	* @brief 构造Replace-SQL语句.
	*
	* @param sTableName 表名
	* @param mpColumns 列名/值对
	* @return string insert-SQL语句
	*/
	string buildReplaceSQL(const string& sTableName, const map<string, pair<FT, string> >& mpColumns);

	/**
	* @brief 构造Update-SQL语句.
	*
	* @param sTableName 表名
	* @param mpColumns 列名/值对
	* @param sCondition where子语句
	* @return string Update-SQL语句
	*/
	string buildUpdateSQL(const string& sTableName, const map<string, pair<FT, string> >& mpColumns, const string& sCondition);

	/**
	* @brief 获取最后执行的SQL语句.
	*
	* @return SQL语句
	*/
	string getLastSQL() { return _sLastSql; }

	/**
	* @brief 获取查询影响数
	* @return int
	*/
	size_t getAffectedRows();
protected:
	/**
	* @brief copy contructor，只申明,不定义,保证不被使用
	*/
	CMysqlHelper(const CMysqlHelper& tcMysql);

	/**
	*
	* @brief 只申明,不定义,保证不被使用
	*/
	CMysqlHelper& operator=(const CMysqlHelper& tcMysql);


private:

	/**
	* 数据库指针
	*/
	MYSQL* _pstMql;

	/**
	* 数据库配置
	*/
	DBConf _dbConf;

	/**
	* 是否已经连接
	*/
	bool _bConnected;

	/**
	* 最后执行的sql
	*/
	string _sLastSql;

};
