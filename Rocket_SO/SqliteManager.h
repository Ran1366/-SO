
#pragma once
#pragma comment(lib,"sqlite3.lib")
#include "sqlite3.h"



#include "CommonManager.h"
// SqliteManager�Ƕ�Sqlite�Ľӿڽ���һ��򵥵ķ�װ��
class SqliteManager
{
public:
	SqliteManager()
		:_db(nullptr)
	{}
	~SqliteManager()
	{
		Close();
	}
	void Open(const std::string &path);
	void Close();
	int ExecuteSql(const std::string &sql);
	void GetTable(const std::string &sql, int &row, int &col, char** &ppRet);
	SqliteManager(const SqliteManager&) = delete;
	SqliteManager& operator=(const SqliteManager&) = delete;
private:
	sqlite3* _db; // ���ݿ����
};

// RAII���Զ��ͷ�sqlite���صĶ�ά����
class AutoGetTable
{
public:
	AutoGetTable(SqliteManager* dbObject, const std::string& sql, int& row, int& col,
		char**& ppRet)
		: _dbObject(dbObject)
		, _ppObject(0)
	{
		_dbObject->GetTable(sql, row, col, ppRet);
		_ppObject = ppRet;
	}
	virtual ~AutoGetTable()
	{
		if (_ppObject)
			sqlite3_free_table(_ppObject);
	}
private:
	AutoGetTable(const AutoGetTable&) = delete;
	AutoGetTable& operator=(const AutoGetTable&) = delete;
private:
	SqliteManager *_dbObject;
	char** _ppObject;
};