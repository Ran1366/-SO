
#include "ScanManager.h"
ScanManager* ScanManager::_inst = nullptr;
std::mutex ScanManager::_mtx;
#define MAXCHANGE 50
void ScanManager::innerScan(const std::string& path)
{
	// �ȶ�  �ļ�ϵͳ�����ݿ�
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
		if (*localit < *dbit) // �����У�����û��->��������
		{
			int ret = DataManager::GetInstance()->InsertDoc(path, *localit);
			if (ret != SQLITE_OK)
			{
				DataManager::GetInstance()->RollbackTransaction();
			}
			++count;
			++localit;
		}
		else if (*localit > *dbit) // ����û�У�������->ɾ������
		{
			int ret = DataManager::GetInstance()->DeleteDoc(path, *dbit);
			if (ret != SQLITE_OK)
			{
				DataManager::GetInstance()->RollbackTransaction();
			}
			++count;
			++dbit;
		}
		else // ���������
		{
			++localit;
			++dbit;
		}
	}

	while (localit != localset.end())
	{
		// ��������
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
		// ɾ������
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

	// �ݹ�ɨ��Ա���Ŀ¼����(to do)
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
	//to do �� �����Scan�ڲ�ֻ����һ�ο���������ύ���������
	//���ڲ�ͳ�ƸĶ���Ŀ
	//ʵ�ָĶ�����ʱ�ع�����
	//����ʹ�ø�����ջ
	//DataManager::GetInstance()->BeginTransaction();
	innerScan(path);
	DataManager::GetInstance()->CommitTransaction();
}