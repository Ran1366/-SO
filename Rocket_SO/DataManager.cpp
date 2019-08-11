#include "CommonManager.h"
#include "DataManager.h"
#include "pinyin.h"

void DataManager::Init()
{
	//加锁(可以换成懒汉模式，解决这个线程安全问题)
	std::unique_lock<std::mutex> lock(_mtx);
	// 打开数据库
	_dbmgr.Open(DOC_DB);
	std::string createtb_sql = "create table if not exists tb_doc (id INTEGER PRIMARY KEY, doc_path text, doc_name text,doc_name_pinyin text, doc_name_initials text)";
	
	_dbmgr.ExecuteSql(createtb_sql);
}
void DataManager::GetDocs(const std::string path, std::set<std::string>& docs)
{
	//id integer primary key autoincrement, doc_path text, doc_name text
	std::string query_sql = "select * from tb_doc where doc_path = ";
	query_sql += "'";
	query_sql += path;
	query_sql += "\'";
	int row;
	int col;
	char ** ppRet;
	//加锁
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.GetTable(query_sql, row, col, ppRet);
	//AutoGetTable agt(&_dbmgr, query_sql, row, col, ppRet);
	//解锁
	lock.unlock();

	std::vector<std::string> tmp;
	for (int i = 2; i <= row + 1; ++i)
	{
		//cout << ppRet[i*col - 1] << " ";
		tmp.push_back(ppRet[i*col - 3]);
		docs.insert(ppRet[i*col - 3]);
	}
	sqlite3_free_table(ppRet);
}
void DataManager::InsertDoc(const std::string path, std::string doc)
{
	//id INTEGER PRIMARY KEY, doc_path text, doc_name text,doc_name_pinyin text, doc_name_initials
	//insert_sql = "insert into tb_doc(doc_path, doc_name) values('stl\', 'vector.h')";
	char insert_sql[1024];
	//create table if not exists tb_doc(id INTEGER PRIMARY KEY, doc_path text, doc_name text, doc_name_pinyin text, doc_name_initials）)";

	std::string pinyin = ChineseConvertPinYinAllSpell(doc);
	std::string initials = ChineseConvertPinYinInitials(doc);
	sprintf(insert_sql, "insert into tb_doc(doc_path, doc_name, doc_name_pinyin,doc_name_initials) values('%s', '%s','%s','%s')",path.c_str(), doc.c_str(),pinyin.c_str(),initials.c_str());
	//sprintf(insert_sql, "insert into tb_doc(doc_path, doc_name) values('%s', '%s')", path.c_str(), doc.c_str());
	std::unique_lock<std::mutex> lock(_mtx);
	insertSql.push_back(insert_sql);
	//cout << insertSql.size() << endl;
	if (insertSql.size() >= 1000)
	{
		//std::unique_lock<std::mutex> lock(_mtx);
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
	//解锁
	lock.unlock();
}
void DataManager::BeginTransaction()
{
	//开始一个事务
	//sqlite3_exec(_db, "begin transaction", 0, 0, &zErrorMsg);
	char begin_sql[128];
	sprintf(begin_sql, "begin transaction");
	//加锁
	//std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(begin_sql);
}
void DataManager::CommitTransaction()
{
	//提交一个事务
	//sqlite3_exec(db, "commit transaction", 0, 0, &zErrorMsg);
	char commit_sql[128];
	sprintf(commit_sql, "commit transaction");
	//加锁
	//std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(commit_sql);
}
void DataManager::RollbackTransaction()
{
	//回滚一个事务
	//sqlite3_exec(db, “rollback transaction”, 0, 0, &zErrorMsg)
	char rollback_sql[128];
	sprintf(rollback_sql, "rollback transaction");
	//加锁
	//std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(rollback_sql);
}
void DataManager::DeleteDoc(const std::string path, std::string doc)
{
	//sql = "DELETE from tb_doc where doc_path = '%s', doc_name = '%s'";
	char delete_sql[1024];
	sprintf(delete_sql, "DELETE from tb_doc where  doc_name = '%s'", doc.c_str());
	//int ret = _dbmgr.ExecuteSql(delete_sql);
	std::unique_lock<std::mutex> lock(_mtx);
	deleteSql.push_back(delete_sql);
	if (deleteSql.size() > 1000)
	{
		//std::unique_lock<std::mutex> lock(_mtx);
		BeginTransaction();
		for (int i = 0; i < deleteSql.size(); ++i)
		{
			int ret = _dbmgr.ExecuteSql(deleteSql[i]);
			if (ret != SQLITE_OK)
			{
				RollbackTransaction();
			}
		}
		deleteSql.clear();
		CommitTransaction();
	}
	//std::unique_unlock<std::mutex> lock(_mtx);
	lock.unlock();
}
void DataManager::Search(const std::string& key, std::vector<std::pair<std::string, std::string>>&
	doc_paths)//使用like模糊查找， 使用拼音搜索
{
	//sql = "SELECT * from tb_doc where doc_path = '%s'";
	char select_sql[1024];
	std::string pinyin = ChineseConvertPinYinAllSpell(key);
	std::string initials = ChineseConvertPinYinInitials(key);
	sprintf(select_sql, "SELECT * from tb_doc where doc_name_pinyin like '%%%s%%' or doc_name_initials like '%%%s%%'", pinyin.c_str(),initials.c_str());
	int row;
	int col;
	char **ppRet;
	//加锁
	std::unique_lock<std::mutex> lock(_mtx);
	//_dbmgr.GetTable(select_sql, row, col, ppRet);
	AutoGetTable agt(&_dbmgr, select_sql, row, col, ppRet);
	//_mtx.unlock();
	/*for (int i = 1; i <= row; ++i)
	{
		for (int j = 0; j < col; ++j)
		{
			cout << ppRet[i*col + j] << " ";
		}
	}*/
	/*cout << "row = " << row << "col = " << col << endl;
	for (int i = 0; i <= row * col; ++i)
	{
		cout << ppRet[i] << endl;
	}*/
	for (int i = 2; i <= row + 1; ++i)
	{
		//cout << ppRet[i*col - 1] << " ";
		//docs.insert(ppRet[i*col - 1]);
		std::pair <std::string, std::string> tmp;
		tmp = std::make_pair(ppRet[i*col - 4], ppRet[i*col - 3]);
		doc_paths.push_back(tmp);
	}
	cout << endl;
}

void DataManager::SplitHighlight(const std::string& str,  std::string& key,
	std::string& prefix, std::string& highlight, std::string& suffix)
{
	//一个GBK汉字要占两个char空间(二字节），而且第一个字节里的值是小于0的。
	//可以据此判断是否为汉字

	//情况一：输入什么就高亮什么(如果有字母，不区分大小写）
	{
		std::string up_str = ToUpper(str);
		std::string up_key = ToUpper(key);
		size_t pos = up_str.find(up_key);
		if (pos != std::string::npos)
		{
			prefix = str.substr(0, pos);
			highlight = str.substr(pos, up_key.size());
			suffix = str.substr(pos + up_key.size(), std::string::npos);
		}
	}

	//情况二：输入的是拼音，高亮对应汉字
	{
		
		std::string str_py = ChineseConvertPinYinAllSpell(str);
		std::string key_py = ChineseConvertPinYinAllSpell(key);
		size_t pos = str_py.find(key_py);

		if (pos != std::string::npos)
		{
			size_t key_start = pos;
			size_t key_end = pos + key_py.size();

			size_t str_i = 0, py_i = 0;
			while (py_i < key_start)
			{
				//如果str的开头就是中文
				if (str[str_i] < 0)
				{
					char chinese[3] = { '\0' };
					chinese[0] = str[str_i];
					chinese[1] = str[str_i + 1];
					str_i += 2;

					std::string py_str = ChineseConvertPinYinAllSpell(chinese);
					py_i += py_str.size();
				}
				else
				{
					++str_i;
					++py_i;
				}
			}

			prefix = str.substr(0, str_i);

			size_t str_j = str_i, py_j = py_i;
			while (py_j < key_end)
			{
				//如果str此时是中文
				if (str[str_j] < 0)
				{
					char chinese[3] = { '\0' };
					chinese[0] = str[str_j];
					chinese[1] = str[str_j + 1];
					str_j += 2;

					std::string py_str = ChineseConvertPinYinAllSpell(chinese);
					py_j += py_str.size();
				}
				else
				{
					++str_j;
					++py_j;
				}
			}

			highlight = str.substr(str_i, str_j - str_i);
			suffix = str.substr(str_j, std::string::npos);
		}

	}

	//情况三：输入的是首字母，高亮对应汉字
	{

		std::string str_init = ChineseConvertPinYinInitials(str);
		std::string key_init = ChineseConvertPinYinInitials(key);
		size_t pos = str_init.find(key_init);

		if (pos != std::string::npos)
		{
			size_t key_start = pos;
			size_t key_end = pos + key_init.size();

			size_t str_i = 0, init_i = 0;
			while (init_i < key_start)
			{
				//如果str的开头就是中文
				if (str[str_i] < 0)
				{
					char chinese[3] = { '\0' };
					chinese[0] = str[str_i];
					chinese[1] = str[str_i + 1];
					str_i += 2;

					std::string py_str = ChineseConvertPinYinInitials(chinese);
					init_i += 1;
				}
				else
				{
					++str_i;
					++init_i;
				}
			}

			prefix = str.substr(0, str_i);

			size_t str_j = str_i, init_j = init_i;
			while (init_j < key_end)
			{
				//如果str此时是中文
				if (str[str_j] < 0)
				{
					char chinese[3] = { '\0' };
					chinese[0] = str[str_j];
					chinese[1] = str[str_j + 1];
					str_j += 2;

					std::string py_str = ChineseConvertPinYinAllSpell(chinese);
					init_j += 1;
				}
				else
				{
					++str_j;
					++init_j;
				}
			}

			highlight = str.substr(str_i, str_j - str_i);
			suffix = str.substr(str_j, std::string::npos);
		}

	}
}
