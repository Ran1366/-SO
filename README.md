


## Roket-SO

## Introduction：

本项目是一个使用C++11编写的文件快速搜索器，用户可输入汉字，拼音或首字母，均能高亮显示搜索结果

|Part I| Part II |Part III |
|--|--|--|
| [文件扫描模块](https://github.com/Ran1366/-SO/blob/master/文件扫描.md) |  [数据管理模块](https://github.com/Ran1366/-SO/blob/master/数据管理.md)  |[逻辑层处理模块](https://github.com/Ran1366/-SO/blob/master/逻辑层处理.md)  |  

## Environment：

OS：Microsoft Windows10

Complier: Visual Studio 2015

## Technical points

 - 使用SQLite3，实现数据的可持久化
 - 使用SQLite3的事务特性，提高数据写入效率
 - 使用单例模式确保程序运行时，仅有一个扫描管理对象和一个数据管理对象
 - 使用RAII技术，将资源进行托管，简化编码
 - 使用互斥锁，解决写入数据库，和读取数据库时的线程安全问题
 - 使用C++11的线程库，多线程扫描磁盘文件
 - 使用高亮匹配算法
