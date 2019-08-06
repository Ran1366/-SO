#pragma once
#include "CommonManager.h"
#include "DataManager.h"

class ScanManager
{
public:
	void innerScan(const std::string& path);
	void Scan(const std::string& path);

	void StartScan()
	{
		//ѭ��ɨ��ָ��Ŀ¼����ʱ˯��
		
		/*std::vector<std::string> localdirs;
		std::vector<std::string> localfiles;
		DirectoryList("E:", localdirs, localfiles);
		int len = localdirs.size();
		std::vector<std::thread> thd;
		for (int i = 0; i < len; ++i)
		{
			thd.push_back(std::thread());
		}
		for (auto& subdirs : localdirs)
		{
			std::string subpath = "E:";
			subpath += '\\';
			subpath += subdirs;
			subdirs = subpath;
		}*/
		
		//Scan("E:");
		/*for (int i = 0; i < len; ++i)
		{
			thd[i] = std::thread(&ScanManager::Scan,CreatInstance(),localdirs[i]);
			thd[i].detach();
		}*/
		//std::thread th(&ScanManager::Scan,this, "E:");
		//th.join();
		while (1)
		{
			Scan("E:");
			Sleep(3000);
		}
		//���ܽ���˳��������ѭ��������
	}

	static ScanManager* CreatInstance()
	{
		if (_inst == nullptr)
		{
			_mtx.lock();
			if (_inst == nullptr)
			{
				_inst = new ScanManager();
			}
			_mtx.unlock();
		}
		return _inst;
	}
private:
	ScanManager()
	{
		//_datamgr.Init();
	}
	static ScanManager *_inst;//��������ָ��
	static std::mutex _mtx;
	//ֻ��һ��ɨ��ģ��
	//DataManager _datamgr;
};
