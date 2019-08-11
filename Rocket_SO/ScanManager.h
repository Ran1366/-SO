#pragma once
#include "CommonManager.h"
#include "DataManager.h"

class ScanManager
{
public:
	void Scan(const std::string& path);

	void StartScan();
	void toThd();

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
	static ScanManager *_inst;//单例对象指针
	static std::mutex _mtx;
	//只需一个扫描模块
	//DataManager _datamgr;
};
