#include "CommonManager.h"
#include "DataManager.h"
#include "pinyin.h"

void DataManager::Init()
{
	//����(���Ի�������ģʽ���������̰߳�ȫ����)
	std::unique_lock<std::mutex> lock(_mtx);
	// �����ݿ�
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
	//����
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.GetTable(query_sql, row, col, ppRet);
	//AutoGetTable agt(&_dbmgr, query_sql, row, col, ppRet);
	//����
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
int DataManager::InsertDoc(const std::string path, std::string doc)
{
	//id INTEGER PRIMARY KEY, doc_path text, doc_name text,doc_name_pinyin text, doc_name_initials
	//insert_sql = "insert into tb_doc(doc_path, doc_name) values('stl\', 'vector.h')";
	char *insert_sql = (char*)malloc(MAXSQLLENGTH);
	//create table if not exists tb_doc(id INTEGER PRIMARY KEY, doc_path text, doc_name text, doc_name_pinyin text, doc_name_initials��)";

	std::string pinyin = ChineseConvertPinYinAllSpell(doc);
	std::string initials = ChineseConvertPinYinInitials(doc);
	sprintf(insert_sql, "insert into tb_doc(doc_path, doc_name, doc_name_pinyin,doc_name_initials) values('%s', '%s','%s','%s')",path.c_str(), doc.c_str(),pinyin.c_str(),initials.c_str());
	//sprintf(insert_sql, "insert into tb_doc(doc_path, doc_name) values('%s', '%s')", path.c_str(), doc.c_str());
	//����
	std::unique_lock<std::mutex> lock(_mtx);
	int ret = _dbmgr.ExecuteSql(insert_sql);
	free(insert_sql);
	return ret;
}
void DataManager::BeginTransaction()
{
	//��ʼһ������
	//sqlite3_exec(_db, "begin transaction", 0, 0, &zErrorMsg);
	char *begin_sql = (char*)malloc(MAXSQLLENGTH);
	sprintf(begin_sql, "begin transaction");
	//����
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(begin_sql);
	free(begin_sql);
}
void DataManager::CommitTransaction()
{
	//�ύһ������
	//sqlite3_exec(db, "commit transaction", 0, 0, &zErrorMsg);
	char *commit_sql = (char*)malloc(MAXSQLLENGTH);
	sprintf(commit_sql, "commit transaction");
	//����
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(commit_sql);
	free(commit_sql);
}
void DataManager::RollbackTransaction()
{
	//�ع�һ������
	//sqlite3_exec(db, ��rollback transaction��, 0, 0, &zErrorMsg)
	char *rollback_sql = (char*)malloc(MAXSQLLENGTH);
	sprintf(rollback_sql, "rollback transaction");
	//����
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(rollback_sql);
	free(rollback_sql);
}
int DataManager::DeleteDoc(const std::string path, std::string doc)
{
	//sql = "DELETE from tb_doc where doc_path = '%s', doc_name = '%s'";
	char *delete_sql = (char*)malloc(MAXSQLLENGTH);
	sprintf(delete_sql, "DELETE from tb_doc where  doc_name = '%s'", doc.c_str());
	int ret = _dbmgr.ExecuteSql(delete_sql);
	free(delete_sql);
	return ret;
}
void DataManager::Search(const std::string& key, std::vector<std::pair<std::string, std::string>>&
	doc_paths)//ʹ��likeģ�����ң� ʹ��ƴ������
{
	//sql = "SELECT * from tb_doc where doc_path = '%s'";
	char *select_sql = (char*)malloc(MAXSQLLENGTH);
	std::string pinyin = ChineseConvertPinYinAllSpell(key);
	std::string initials = ChineseConvertPinYinInitials(key);
	sprintf(select_sql, "SELECT * from tb_doc where doc_name_pinyin like '%%%s%%' or doc_name_initials like '%%%s%%'", pinyin.c_str(),initials.c_str());
	int row;
	int col;
	char **ppRet;
	//����
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.GetTable(select_sql, row, col, ppRet);
	//AutoGetTable agt(&_dbmgr, select_sql, row, col, ppRet);
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
	sqlite3_free_table(ppRet);
	free(select_sql);
	cout << endl;
}

void DataManager::SplitHighlight(const std::string& str,  std::string& key,
	std::string& prefix, std::string& highlight, std::string& suffix)
{
	//һ��GBK����Ҫռ����char�ռ�(���ֽڣ������ҵ�һ���ֽ����ֵ��С��0�ġ�
	//���Ծݴ��ж��Ƿ�Ϊ����

	//���һ������ʲô�͸���ʲô(�������ĸ�������ִ�Сд��
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

	//��������������ƴ����������Ӧ����
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
				//���str�Ŀ�ͷ��������
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
				//���str��ʱ������
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

	//������������������ĸ��������Ӧ����
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
				//���str�Ŀ�ͷ��������
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
				//���str��ʱ������
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