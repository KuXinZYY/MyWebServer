#include "fileutil.h"

///******************************************************************************//
AppendFile::AppendFile(std::string filename)
    : fp_(fopen(filename.c_str() , "ae"))//'e' for O_CLOEXEC , 'a'表示要么追加写入存在的文件,要么创建文件并追加.
{
    setbuffer(fp_ , buffer_ , sizeof(buffer_));//设置用户写缓冲区,仅仅写入缓冲区,写满或者fflush后通过write写入内核缓冲区
}

AppendFile::~AppendFile()
{
    fclose(fp_);//关闭fp_并将用户缓冲区数据全部刷新到内核缓冲区等待写入磁盘
}

void AppendFile::flush()
{
    assert(fflush(fp_) == 0);
}
void AppendFile::append(const char *logline , const size_t len)
{
    size_t n = write(logline , len);//返回写入用户缓冲区的前多少字节.
    size_t remain = len - n;//如果缓冲区已满,则继续写,直到写完.
    while(remain > 0){//直到remain为0,则全部写入用户缓冲区.
        size_t x = write(logline + n , remain);
        if(x == 0){//返回０，则应该是有错误
            int err = ferror(fp_);
            if(err){
                fprintf(stderr , "AppendFile::append() failed %s!\n" , strerror(err));//向标准错误写
            }
            break;
        }
        n = n + x;//记录已经写入了多少
        remain = remain - x;//剩余大小字节
    }
}
size_t AppendFile::write(const char *logline , size_t len)
{
    return fwrite_unlocked(logline, 1, len, fp_);
}
