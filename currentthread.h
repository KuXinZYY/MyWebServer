#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

/*
定义当前线程数据缓存类
*/
namespace CurrentThread
{
    // internal
    extern __thread int t_cachedTid;//为了避免多次系统调用,缓存tid
    extern __thread char t_tidString[32];//tid的string类型,便于日志输出
    extern __thread int t_tidStringLength;//string类型长度
    extern __thread const char* t_threadName;//线程的名字
    void cacheTid();//获取tid函数

    /*
        以下都是内联函数哦
    */
    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char* tidString()// for logging
    {
        return t_tidString;
    }

    inline int tidStringLength()// for logging
    {
        return t_tidStringLength;
    }

    inline const char* name()
    {
        return t_threadName;
    }
}


#endif // CURRENTTHREAD_H
