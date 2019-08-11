
#include "ScanManager.h"
ScanManager* ScanManager::_inst = nullptr;
std::mutex ScanManager::_mtx;
#define MAXCHANGE 1000
void ScanManager::Scan(const std::string& path)
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
	//static int count = 0;
	while (localit != localset.end() && dbit != dbset.end())
	{
		if (*localit < *dbit) // 本地有，数据没有->新增数据
		{
			DataManager::GetInstance()->InsertDoc(path, *localit);
			/*if (ret != SQLITE_OK)
			{
				DataManager::GetInstance()->RollbackTransaction();
			}
			++count;*/
			++localit;
		}
		else if (*localit > *dbit) // 本地没有，数据有->删除数据
		{
			DataManager::GetInstance()->DeleteDoc(path, *dbit);
			/*if (ret != SQLITE_OK)
			{
				DataManager::GetInstance()->RollbackTransaction();
			}
			++count;*/
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
		DataManager::GetInstance()->InsertDoc(path, *localit);
		/*if (ret != SQLITE_OK)
		{
			DataManager::GetInstance()->RollbackTransaction();
		}
		++count;*/
		++localit;
	}

	while (dbit != dbset.end())
	{
		// 删除数据
		DataManager::GetInstance()->DeleteDoc(path, *dbit);
		/*if (ret != SQLITE_OK)
		{
			DataManager::GetInstance()->RollbackTransaction();
		}
		++count;*/
		++dbit;
	}
	/*if (count >= MAXCHANGE)
	{
		DataManager::GetInstance()->CommitTransaction();
		count = 0;
	}*/

	// 递归扫描对比子目录数据(to do)
	for (const auto& subdirs : localdirs)
	{
		std::string subpath = path;
		subpath += '\\';
		subpath += subdirs;

		Scan(subpath);
	}
}

void ScanManager::toThd()
{
	while (1)
	{
		std::vector<std::string> localdirs;
		std::vector<std::string> localfiles;
		std::string path = "F:";
		DirectoryList(path, localdirs, localfiles);
		int len = localdirs.size();
		std::vector<std::thread> thd_list;
		thd_list.resize(len);
		for (auto& subdirs : localdirs)
		{
			std::string subpath = path;
			subpath += '\\';
			subpath += subdirs;
			subdirs = subpath;
		}
		//DataManager::GetInstance()->BeginTransaction();
		for (int i = 0; i < len; ++i)
		{
			std::thread thd(&ScanManager::Scan, this, localdirs[i]);
			thd_list[i] = std::move(thd);
		}
		//cout << "扫描中，请耐心等待..." << endl;
		for (int i = 0; i < len; ++i)
		{
			thd_list[i].join();
		}
		//如果扫描完成后有部分数据未同步
		DataManager::GetInstance()->Aftermath();
		Sleep(5000);
	}
}
void ScanManager::StartScan()
{
	std::thread scan_thd(&ScanManager::toThd, this);
	scan_thd.detach();
}