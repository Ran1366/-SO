#pragma once
#include "CommonManager.h"
#include "SqliteManager.h"
#include <io.h>
#include <set>
//posix标准:使用其中_find...接口,(好处是在Linux下和windows下都能使用
//定义如下:
//long _findfirst( char *filespec, struct _finddata_t *fileinfo );
//int _findnext( long handle, struct _finddata_t *fileinfo ); 
//int _findclose( long handle ); 
//其中_finddata_t结构体定义如下:
//struct _finddata64i32_t {
//	unsigned    attrib;
//	__time64_t  time_create;    /* -1 for FAT file systems */
//	__time64_t  time_access;    /* -1 for FAT file systems */
//	__time64_t  time_write;
//	_fsize_t    size;
//	char        name[260];
//}
#define DOC_DB "docs.db"

static std::string ToUpper(const std::string &str)
{
	int i = 0;
	std::string tmp = str;
	while (tmp[i])
	{
		if (tmp[i] < 0)
		{
			i += 2;
		}
		else
		{
			tmp[i] = toupper(tmp[i]);
			++i;
		}
	}
	return tmp;
}

static void DirectoryList(std::string path, std::vector<std::string> &subdirs,
	std::vector<std::string> &subfiles)
{
	_finddata64i32_t file;
	std::string _path = path + "\\*.*";
	long handle = _findfirst(_path.c_str(), &file);
	if (handle == -1)
	{
		ERROE_LOG("_findfirst:%s", path.c_str());
	}
	else
	{
		TRACE_LOG("_findfirst success:%s", path.c_str());
	}
	do
	{
		//#define _A_NORMAL 0x00 // Normal file - No read/write restrictions
		//#define _A_RDONLY 0x01 // Read only file
		//#define _A_HIDDEN 0x02 // Hidden file
		//#define _A_SYSTEM 0x04 // System file
		//#define _A_SUBDIR 0x10 // Subdirectory
		//#define _A_ARCH   0x20 // Archive file
		if ((file.attrib & _A_SUBDIR) && !(file.attrib & _A_RDONLY)
			&& !(file.attrib & _A_HIDDEN) && !(file.attrib & _A_SYSTEM)
			&& !(file.attrib & _A_ARCH))
		{
			if (strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
			{
				subdirs.push_back(file.name);
				std::string _path = path;
				_path += "\\";
				_path += file.name;
				//DirectoryList(_path, subdirs, subfiles);
			}
		}
		else if(!(file.attrib & _A_SUBDIR))
		{
			subfiles.push_back(file.name);
		}
	} while (_findnext(handle, &file) == 0);


	_findclose(handle);
}

class DataManager
{
public:
	~DataManager()
	{
		_dbmgr.Close();
	}
	static DataManager* DataManager::GetInstance()
	{
		static DataManager inst;
		return &inst;
	}
	void Init();
	void GetDocs(const std::string path, std::set<std::string>& docs);
	void InsertDoc(const std::string path, std::string doc);
	void DeleteDoc(const std::string path, std::string doc);
	void Search(const std::string& key, std::vector<std::pair<std::string, std::string>>&
		doc_paths);
	void SplitHighlight(const std::string& str, std::string& key,
		std::string& prefix, std::string& highlight, std::string& suffix);
	void BeginTransaction();
	void CommitTransaction();
	void RollbackTransaction();
	void Aftermath()
	{
		if (!insertSql.empty())
		{
			BeginTransaction();
			for (int i = 0; i < insertSql.size(); ++i)
			{
				//cout << "inserting...  " << i << endl;
				int ret = _dbmgr.ExecuteSql(insertSql[i]);
				if (ret != SQLITE_OK)
				{
					RollbackTransaction();
				}
			}
			insertSql.clear();
			CommitTransaction();
		}
		if (!deleteSql.empty())
		{
			BeginTransaction();
			for (int i = 0; i < deleteSql.size(); ++i)
			{
				//cout << "inserting...  " << i << endl;
				int ret = _dbmgr.ExecuteSql(deleteSql[i]);
				if (ret != SQLITE_OK)
				{
					RollbackTransaction();
				}
			}
			deleteSql.clear();
			CommitTransaction();
		}
	}
private:
	DataManager()
	{
		insertSql.reserve(1000);
		deleteSql.reserve(1000);
	}
	DataManager(const DataManager&) = delete;
	DataManager operator=(const DataManager&) = delete;
	std::vector<std::string> insertSql;
	std::vector<std::string> deleteSql;
	SqliteManager _dbmgr;
	std::mutex _mtx;
};
