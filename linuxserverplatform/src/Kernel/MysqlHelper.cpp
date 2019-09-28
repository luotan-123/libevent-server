//mysqlhelper.cpp
#include "CommonHead.h"
#include "MysqlHelper.h"
#include <string.h>
#include <sstream>

using namespace std;

CMysqlHelper::CMysqlHelper() :_bConnected(false)
{
	_pstMql = mysql_init(NULL);
}

CMysqlHelper::CMysqlHelper(const string& sHost, const string& sUser, const string& sPasswd, const string& sDatabase, const string& sCharSet, int port, int iFlag)
	: _bConnected(false)
{
	init(sHost, sUser, sPasswd, sDatabase, sCharSet, port, iFlag);

	_pstMql = mysql_init(NULL);
}

CMysqlHelper::CMysqlHelper(const DBConf& tcDBConf)
	:_bConnected(false)
{
	_dbConf = tcDBConf;

	_pstMql = mysql_init(NULL);
}

CMysqlHelper::~CMysqlHelper()
{
	if (_pstMql != NULL)
	{
		mysql_close(_pstMql);
		_pstMql = NULL;
	}
}

void CMysqlHelper::init(const string& sHost, const string& sUser, const string& sPasswd, const string& sDatabase, const string& sCharSet, int port, int iFlag)
{
	_dbConf._host = sHost;
	_dbConf._user = sUser;
	_dbConf._password = sPasswd;
	_dbConf._database = sDatabase;
	_dbConf._charset = sCharSet;
	_dbConf._port = port;
	_dbConf._flag = iFlag;
}

void CMysqlHelper::init(const DBConf& tcDBConf)
{
	_dbConf = tcDBConf;
}

void CMysqlHelper::connect()
{
	disconnect();

	if (_pstMql == NULL)
	{
		_pstMql = mysql_init(NULL);
	}

	//建立连接后, 自动调用设置字符集语句
	if (!_dbConf._charset.empty()) {
		if (mysql_options(_pstMql, MYSQL_SET_CHARSET_NAME, _dbConf._charset.c_str())) {
			throw MysqlHelper_Exception(string("CMysqlHelper::connect: mysql_options MYSQL_SET_CHARSET_NAME ") + _dbConf._charset + ":" + string(mysql_error(_pstMql)));
		}
	}

	if (mysql_real_connect(_pstMql, _dbConf._host.c_str(), _dbConf._user.c_str(), _dbConf._password.c_str(), _dbConf._database.c_str(), _dbConf._port, NULL, _dbConf._flag) == NULL)
	{
		throw MysqlHelper_Exception("[CMysqlHelper::connect]: mysql_real_connect: " + string(mysql_error(_pstMql)));
	}

	_bConnected = true;
}

void CMysqlHelper::disconnect()
{
	if (_pstMql != NULL)
	{
		mysql_close(_pstMql);
		_pstMql = mysql_init(NULL);
	}

	_bConnected = false;
}

string CMysqlHelper::escapeString(const string& sFrom)
{
	if (!_bConnected)
	{
		connect();
	}

	string sTo;
	string::size_type iLen = sFrom.length() * 2 + 1;
	char* pTo = (char*)malloc(iLen);

	memset(pTo, 0x00, iLen);

	mysql_real_escape_string(_pstMql, pTo, sFrom.c_str(), sFrom.length());

	sTo = pTo;

	free(pTo);

	return sTo;
}

MYSQL* CMysqlHelper::getMysql(void)
{
	return _pstMql;
}

string CMysqlHelper::buildInsertSQL(const string& sTableName, const RECORD_DATA& mpColumns)
{
	ostringstream sColumnNames;
	ostringstream sColumnValues;

	map<string, pair<FT, string> >::const_iterator itEnd = mpColumns.end();

	for (map<string, pair<FT, string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
	{
		if (it == mpColumns.begin())
		{
			sColumnNames << "`" << it->first << "`";
			if (it->second.first == DB_INT)
			{
				sColumnValues << it->second.second;
			}
			else
			{
				sColumnValues << "'" << escapeString(it->second.second) << "'";
			}
		}
		else
		{
			sColumnNames << ",`" << it->first << "`";
			if (it->second.first == DB_INT)
			{
				sColumnValues << "," + it->second.second;
			}
			else
			{
				sColumnValues << ",'" + escapeString(it->second.second) << "'";
			}
		}
	}

	ostringstream os;
	os << "insert into " << sTableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";
	return os.str();
}

string CMysqlHelper::buildReplaceSQL(const string& sTableName, const RECORD_DATA& mpColumns)
{
	ostringstream sColumnNames;
	ostringstream sColumnValues;

	map<string, pair<FT, string> >::const_iterator itEnd = mpColumns.end();
	for (map<string, pair<FT, string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
	{
		if (it == mpColumns.begin())
		{
			sColumnNames << "`" << it->first << "`";
			if (it->second.first == DB_INT)
			{
				sColumnValues << it->second.second;
			}
			else
			{
				sColumnValues << "'" << escapeString(it->second.second) << "'";
			}
		}
		else
		{
			sColumnNames << ",`" << it->first << "`";
			if (it->second.first == DB_INT)
			{
				sColumnValues << "," + it->second.second;
			}
			else
			{
				sColumnValues << ",'" << escapeString(it->second.second) << "'";
			}
		}
	}

	ostringstream os;
	os << "replace into " << sTableName << " (" << sColumnNames.str() << ") values (" << sColumnValues.str() << ")";
	return os.str();
}

string CMysqlHelper::buildUpdateSQL(const string& sTableName, const RECORD_DATA& mpColumns, const string& sWhereFilter)
{
	ostringstream sColumnNameValueSet;

	map<string, pair<FT, string> >::const_iterator itEnd = mpColumns.end();

	for (map<string, pair<FT, string> >::const_iterator it = mpColumns.begin(); it != itEnd; ++it)
	{
		if (it == mpColumns.begin())
		{
			sColumnNameValueSet << "`" << it->first << "`";
		}
		else
		{
			sColumnNameValueSet << ",`" << it->first << "`";
		}

		if (it->second.first == DB_INT)
		{
			sColumnNameValueSet << "= " << it->second.second;
		}
		else
		{
			sColumnNameValueSet << "= '" << escapeString(it->second.second) << "'";
		}
	}

	ostringstream os;
	os << "update " << sTableName << " set " << sColumnNameValueSet.str() << " " << sWhereFilter;

	return os.str();
}

string CMysqlHelper::getVariables(const string& sName)
{
	string sql = "SHOW VARIABLES LIKE '" + sName + "'";

	MysqlData data;
	queryRecord(sql, data);

	if (data.size() == 0)
	{
		return "";
	}

	if (sName == data[0]["Variable_name"])
	{
		return data[0]["Value"];
	}

	return "";
}

void CMysqlHelper::execute(const string& sSql)
{
	/**
	没有连上, 连接数据库
	*/
	if (!_bConnected)
	{
		connect();
	}

	_sLastSql = sSql;

	int iRet = mysql_real_query(_pstMql, sSql.c_str(), sSql.length());
	if (iRet != 0)
	{
		/**
		自动重新连接
		*/
		int iErrno = mysql_errno(_pstMql);
		if (iErrno == 2013 || iErrno == 2006)
		{
			connect();
			iRet = mysql_real_query(_pstMql, sSql.c_str(), sSql.length());
		}
	}

	if (iRet != 0)
	{
		throw MysqlHelper_Exception("[CMysqlHelper::execute]: mysql_query: [ " + sSql + " ] :" + string(mysql_error(_pstMql)));
	}
}

bool CMysqlHelper::queryRecord(const string& sSql, MysqlData& data, bool bCharacterSet/* = false*/)
{
	/**
	没有连上, 连接数据库
	*/
	if (!_bConnected)
	{
		connect();
	}

	if (bCharacterSet)
	{
		MYSQL_RES* pRes;
		mysql_query(_pstMql, "SET NAMES UTF8"); //设置编码格式 Windows： SET NAMES GBK，Linux ： SET NAMES UTF8
		do
		{
			pRes = mysql_use_result(_pstMql);
			mysql_free_result(pRes);
		} while (!mysql_next_result(_pstMql));
	}

	_sLastSql = sSql;

	int iRet = mysql_real_query(_pstMql, sSql.c_str(), sSql.length());
	if (iRet != 0)
	{
		/**
		自动重新连接
		*/
		int iErrno = mysql_errno(_pstMql);
		if (iErrno == 2013 || iErrno == 2006)
		{
			connect();
			iRet = mysql_real_query(_pstMql, sSql.c_str(), sSql.length());
		}
	}

	if (iRet != 0)
	{
		throw MysqlHelper_Exception("[CMysqlHelper::execute]: mysql_query: [ " + sSql + " ] :" + string(mysql_error(_pstMql)));
	}

	MYSQL_RES* pstRes = mysql_store_result(_pstMql);

	if (pstRes == NULL)
	{
		throw MysqlHelper_Exception("[CMysqlHelper::queryRecord]: mysql_store_result: " + sSql + " : " + string(mysql_error(_pstMql)));
	}

	vector<string> vtFields;
	MYSQL_FIELD* field;
	while ((field = mysql_fetch_field(pstRes)))
	{
		vtFields.push_back(field->name);
	}

	map<string, string> mpRow;
	MYSQL_ROW stRow;

	while ((stRow = mysql_fetch_row(pstRes)) != (MYSQL_ROW)NULL)
	{
		mpRow.clear();
		unsigned long* lengths = mysql_fetch_lengths(pstRes);
		for (size_t i = 0; i < vtFields.size(); i++)
		{
			if (stRow[i])
			{
				mpRow[vtFields[i]] = string(stRow[i], lengths[i]);
			}
			else
			{
				mpRow[vtFields[i]] = "";
			}
		}

		data.data().push_back(mpRow);
	}

	mysql_free_result(pstRes);

	return true;
}

void CMysqlHelper::sqlExec(const char* sql, bool bSetGBK/* = false*/)
{
	/**
	没有连上, 连接数据库
	*/
	if (!_bConnected)
	{
		connect();
	}

	if (bSetGBK)
	{
		MYSQL_RES* pRes;
		mysql_query(_pstMql, "SET NAMES GBK"); //设置编码格式（SET NAMES GBK也行），否则cmd下中文乱码  
		do
		{
			pRes = mysql_use_result(_pstMql);
			mysql_free_result(pRes);
		} while (!mysql_next_result(_pstMql));
	}

	_sLastSql = sql;

	int iRet = mysql_real_query(_pstMql, sql, strlen(sql));
	if (iRet != 0)
	{
		/**
		自动重新连接
		*/
		int iErrno = mysql_errno(_pstMql);
		if (iErrno == 2013 || iErrno == 2006)
		{
			connect();
			iRet = mysql_real_query(_pstMql, sql, strlen(sql));
		}
	}

	if (iRet != 0)
	{
		throw MysqlHelper_Exception("[CMysqlHelper::execute]: mysql_query: [ " + string(sql) + " ] :" + string(mysql_error(_pstMql)));
	}

	MYSQL_RES* pRes = NULL;
	do
	{
		pRes = mysql_use_result(_pstMql);
		if (pRes == NULL)
		{
			return;
		}
		mysql_free_result(pRes);
	} while (!mysql_next_result(_pstMql));
}

size_t CMysqlHelper::updateRecord(const string& sTableName, const RECORD_DATA& mpColumns, const string& sCondition)
{
	string sSql = buildUpdateSQL(sTableName, mpColumns, sCondition);
	execute(sSql);

	return (size_t)mysql_affected_rows(_pstMql);
}

size_t CMysqlHelper::insertRecord(const string& sTableName, const RECORD_DATA& mpColumns)
{
	string sSql = buildInsertSQL(sTableName, mpColumns);
	execute(sSql);

	return (size_t)mysql_affected_rows(_pstMql);
}

size_t CMysqlHelper::replaceRecord(const string& sTableName, const RECORD_DATA& mpColumns)
{
	string sSql = buildReplaceSQL(sTableName, mpColumns);
	execute(sSql);

	return (size_t)mysql_affected_rows(_pstMql);
}

size_t CMysqlHelper::deleteRecord(const string& sTableName, const string& sCondition)
{
	ostringstream sSql;
	sSql << "delete from " << sTableName << " " << sCondition;

	execute(sSql.str());

	return (size_t)mysql_affected_rows(_pstMql);
}

size_t CMysqlHelper::getRecordCount(const string& sTableName, const string& sCondition)
{
	ostringstream sSql;
	sSql << "select count(*) as num from " << sTableName << " " << sCondition;

	MysqlData data;
	queryRecord(sSql.str(), data);

	long n = atol(data[0]["num"].c_str());

	return n;

}

size_t CMysqlHelper::getSqlCount(const string& sCondition)
{
	ostringstream sSql;
	sSql << "select count(*) as num " << sCondition;

	MysqlData data;
	queryRecord(sSql.str(), data);

	long n = atol(data[0]["num"].c_str());

	return n;
}

int CMysqlHelper::getMaxValue(const string& sTableName, const string& sFieldName, const string& sCondition)
{
	ostringstream sSql;
	sSql << "select " << sFieldName << " as f from " << sTableName << " " << sCondition << " order by f desc limit 1";

	MysqlData data;
	queryRecord(sSql.str(), data);

	int n = 0;

	if (data.size() == 0)
	{
		n = 0;
	}
	else
	{
		n = atoi(data[0]["f"].c_str());
	}

	return n;
}

long CMysqlHelper::lastInsertID()
{
	return (long)mysql_insert_id(_pstMql);
}

size_t CMysqlHelper::getAffectedRows()
{
	return (size_t)mysql_affected_rows(_pstMql);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

vector<map<string, string> >& CMysqlHelper::MysqlData::data()
{
	return _data;
}

size_t CMysqlHelper::MysqlData::size()
{
	return _data.size();
}

map<string, string>& CMysqlHelper::MysqlData::operator[](size_t i)
{
	return _data[i];
}