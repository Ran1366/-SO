
#include "ScanManager.h"
ScanManager* ScanManager::_inst = nullptr;
std::mutex ScanManager::_mtx;
#define MAXCHANGE 50
void ScanManager::innerScan(const std::string& path)
{
	// 比对  文件系统和数据库
	std::vector<std::string> localdirs;
	std::vector<std::string> localfiles;
	DirectoryList(path, localdirs, localfiles);

	std::set<std::string> localset;
	localset.insert(localfiles.begin(), localfiles.end());
	localset.insert(localdirs.begin(), localdirs.end());

	std::set<std::string> dbset;
	DataManager::GetInstance()->GetDocs(path, dbset);

	auto localit = localset.begin();
	auto dbit = dbset.begin();
	static int count = 0;
	while (localit != localset.end() && dbit != dbset.end())
	{
		if (*localit < *dbit) // 本地有，数据没有->新增数据
		{
			int ret = DataManager::GetInstance()->InsertDoc(path, *localit);
			if (ret != SQLITE_OK)
			{
				DataManager::GetInstance()->RollbackTransaction();
			}
			++count;
			++localit;
		}
		else if (*localit > *dbit) // 本地没有，数据有->删除数据
		{
			int ret = DataManager::GetInstance()->DeleteDoc(path, *dbit);
			if (ret != SQLITE_OK)
			{
				DataManager::GetInstance()->RollbackTransaction();
			}
			++count;
			++dbit;
		}
		else // 不变的数据
		{
			++localit;
			++dbit;
		}
	}

	while (localit != localset.end())
	{
		// 新增数据
		int ret = DataManager::GetInstance()->InsertDoc(path, *localit);
		if (ret != SQLITE_OK)
		{
			DataManager::GetInstance()->RollbackTransaction();
		}
		++count;
		++localit;
	}

	while (dbit != dbset.end())
	{
		// 删除数据
		int ret = DataManager::GetInstance()->DeleteDoc(path, *dbit);
		if (ret != SQLITE_OK)
		{
			DataManager::GetInstance()->RollbackTransaction();
		}
		++count;
		++dbit;
	}
	if (count >= MAXCHANGE)
	{
		DataManager::GetInstance()->CommitTransaction();
		count = 0;
	}

	// 递归扫描对比子目录数据(to do)
	for (const auto& subdirs : localdirs)
	{
		std::string subpath = path;
		subpath += '\\';
		subpath += subdirs;

		innerScan(subpath);
	}
}

void ScanManager::Scan(const std::string& path)
{
	//to do ： 解决在Scan内部只调用一次开启事务和提交事务的问题
	//在内部统计改动条目
	//实现改动出错时回滚事务
	//考虑使用辅助的栈
	//DataManager::GetInstance()->BeginTransaction();
	innerScan(path);
	DataManager::GetInstance()->CommitTransaction();
}