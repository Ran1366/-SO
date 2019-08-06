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
		if (file.attrib & _A_SUBDIR)
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
		else
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
	int InsertDoc(const std::string path, std::string doc);
	int DeleteDoc(const std::string path, std::string doc);
	void Search(const std::string& key, std::vector<std::pair<std::string, std::string>>&
		doc_paths);
	void SplitHighlight(const std::string& str, std::string& key,
		std::string& prefix, std::string& highlight, std::string& suffix);
	void BeginTransaction();
	void CommitTransaction();
	void RollbackTransaction();
	
private:
	DataManager()
	{
	}
	DataManager(const DataManager&) = delete;
	DataManager operator=(const DataManager&) = delete;
	SqliteManager _dbmgr;
	std::mutex _mtx;
};