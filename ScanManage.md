


# 文件扫描


## Posix标准的目录检索接口


    long _findfirst( char *filespec, struct _finddata_t *fileinfo );
    int _findnext( long handle, struct _finddata_t *fileinfo ); 
    int _findclose( long handle );

用上面的接口实现目录检索功能，本项目使用的是深度优先遍历，会将指定目录下的所有文件都扫描出来
使用这套接口的好处是可以将程序移植到Linux中，如果需要在Linux下支持拼音搜索则需要将UTF-8转换为GDB
## 数据同步
文件扫描主要是获取本地目录下的所有文件与数据库中的表项做对比：

 - 如果本地存在而数据库不存在，则在数据库中添加该文件
 - 如果本地不存在而数据库中存在，则在数据库中删除该文件
 - 如果本地存在数据库中也存在，则进行下一个文件的比对

以此进行同步
