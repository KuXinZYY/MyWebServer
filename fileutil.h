#ifndef FILEUTIL_H
#define FILEUTIL_H

#include<string>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<memory>
#include<mutexlock.h>
#include<assert.h>

/*
 * 用于创建或者打开文件,然后添加数据行都末尾
 * 封装底层的fwrite和fflush
 * 数据流向从用户缓冲区->内核缓冲区->磁盘数据
*/
class AppendFile
{
public:
    explicit AppendFile(std::string filename = std::string("log"));
    ~AppendFile();
    void flush();//将用户缓冲区刷新到内核缓冲区.
    void append(const char *logline , const size_t len);//添加到文件末尾,刷新到用户缓冲区.
private:
    AppendFile(AppendFile &rth);//禁止拷贝
    AppendFile &operator=(AppendFile &rth);//禁止赋值
    size_t write(const char *logline , size_t len);//内部实际的fflush操作
    FILE *fp_;//文件流指针,指向打开的文件.
    char buffer_[64*1024];//stdio用户缓冲区 64kb
};

#endif // FILEUTIL_H
