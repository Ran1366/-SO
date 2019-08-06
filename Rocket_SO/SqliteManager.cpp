#include "SqliteManager.h"


void SqliteManager::Open(const std::string &path)
{
	//sqlite3_open(const char *filename, sqlite3 **ppDb)
	int ret = sqlite3_open(path.c_str(), &_db);
	if (ret != SQLITE_OK)
	{
		ERROE_LOG("SqliteManager::sqlite3_open failed:%s", path.c_str());
	}
	else
	{
		TRACE_LOG("SqliteManager::sqlite3_open success:%s", path.c_str());
	}
}
void SqliteManager::Close()
{
	//sqlite3_close(sqlite3*)
	int ret = sqlite3_close(_db);
	if (ret != SQLITE_OK)
	{
		ERROE_LOG("SqliteManager::sqlite3_close failed");
	}
	else
	{
		TRACE_LOG("SqliteManager::sqlite3_close success");
	}
}
int SqliteManager::ExecuteSql(const std::string &sql)
{
	//sqlite3_exec(sqlite3*, const char *sql, sqlite_callback,
	//				void *data, char **errmsg)
	//int sqlite3_exec(
	//	sqlite3*,                                  /* An open database */
	//	const char *sql,                           /* SQL to be evaluated */
	//	int(*callback)(void*, int, char**, char**),  /* Callback function */
	//	void *,                                    /* 1st argument to callback */
	//	char **errmsg                              /* Error msg written here */
	//);
	char *errmsg;
	int ret = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errmsg);
	if (ret != SQLITE_OK)
	{
		ERROE_LOG("SqliteManager::sqlite3_exec failed:%s", sql.c_str());
	}
	else
	{
		TRACE_LOG("SqliteManager::sqlite3_exec success:%s", sql.c_str());
	}
	return ret;
}
void SqliteManager::GetTable(const std::string &sql, int &row, int &col, char** &ppRet)
{
	//int sqlite3_get_table(
	//	sqlite3 *db,          /* An open database */
	//	const char *zSql,     /* SQL to be evaluated */
	//	char ***pazResult,    /* Results of the query */
	//	int *pnRow,           /* Number of result rows written here */
	//	int *pnColumn,        /* Number of result columns written here */
	//	char **pzErrmsg       /* Error msg written here */
	//);
	//void sqlite3_free_table(char **result);
	char *pzErrmsg;
	int ret = sqlite3_get_table(_db, sql.c_str(), &ppRet, &row, &col, &pzErrmsg);
	if (ret != SQLITE_OK)
	{
		ERROE_LOG("SqliteManager::sqlite3_exec failed:%s", sql.c_str());
	}
	else
	{
		TRACE_LOG("SqliteManager::sqlite3_exec success:%s", sql.c_str());
	}
}
