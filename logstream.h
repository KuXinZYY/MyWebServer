#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include<memory.h>
#include<string>
#include<algorithm>

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;


/*
 * 简单封装一个固定大小的缓冲区,通过模板参数传入参数确定缓冲区的大小
 */
template<int SIZE>
class FixedBuffer
{
public:
    FixedBuffer()
    : cur_(data_)
    {

    }
    ~FixedBuffer(){}

    void append(const char *buf , size_t len)//将buf中的内容添加到缓冲区
    {
        if(avail() > static_cast<int>(len)){
            memmove(cur_ , buf , len);//防止内存重叠,很有效
            cur_ += len;//指向后续指针
        }

    }
    const char *data() const//缓冲区首地址
    {
        return data_;
    }

    int length() const//缓冲区中已有数据长度
    {
        return static_cast<int>(cur_ - data_);
    }

    char *current()//数据指针当前位置
    {
        return cur_;
    }

    int avail()//缓冲区剩余可用空间
    {
        return (SIZE - length());
    }

    void add(size_t len)//数据指针前移动len位
    {
        cur_ += len;
    }
    void reset()//缓冲区数据指针复位
    {
        cur_ = data_;
    }
    void bzero()//缓冲区数据清0
    {
        memset(data_ , 0 , sizeof(data_));
    }

private:
    FixedBuffer(FixedBuffer &rth);//禁止拷贝
    FixedBuffer &operator=(FixedBuffer &rth);//禁止赋值
    char data_[SIZE];//简单的缓冲区大小
    char *cur_;//指向当前没有使用的缓冲区首部位置
};


/*
 * 此类主要用于操作符的重载,使得日志可以如同 log<<使用比较简单.
 *
 */
class LogStream
{
public:
    //1.函数重载运算符并实现多态.返回引用,完全是为了 log<<12<<er等等操作
    //2.区分reinterpret_cast与static_cast
    /*
        c类型转换:过于粗鲁区分度不高,并且难以识别,用下面的四种可以明确转换的用途.
        static_cast 基本和C风格强制转换一样强大.但是不能将struct转换成int或者double转换成指针,且不能从表达式中去除const属性
        const_cast 最普通用途就是将对象的const属性去掉.
        dynamic_cast 安全的沿着类的继承关系向下转换.将基类指针或引用安全地转换成派生类指针或引用.转换失败则返回NULL,或抛出异常
        reinterpret_cast 普通用于就是用于函数指针类型之间的转换.

    */
    typedef FixedBuffer<kSmallBuffer> Buffer;//一条消息最长4kb
    LogStream()
    {
    }
    ~LogStream(){}

    void append(const char* data, int len)//将数据添加到buffer_中
    {
        buffer_.append(data, len);
    }

    const Buffer& buffer() const//获取buffer_引用
    {
        return buffer_;
    }

    void resetBuffer()//缓冲区重置
    {
        buffer_.reset();
    }

    LogStream& operator<<(bool val)
    {
        buffer_.append((val ? "1" : "0") , 1);//bool类型通过1或者0表示
        return *this;
    }

    LogStream& operator<<(short val)
    {
        formatInteger(val);
        return *this;
    }
    LogStream& operator<<(unsigned short val)
    {
        formatInteger(val);
        return *this;
    }

    LogStream& operator<<(int val)
    {
        formatInteger(val);
        return *this;
    }
    LogStream& operator<<(unsigned int val)
    {
        formatInteger(val);
        return *this;
    }

    LogStream& operator<<(long val)
    {
        formatInteger(val);
        return *this;
    }
    LogStream& operator<<(unsigned long val)
    {
        formatInteger(val);
        return *this;
    }

    LogStream& operator<<(long long val)
    {
        formatInteger(val);
        return *this;
    }
    LogStream& operator<<(unsigned long long val)
    {
        formatInteger(val);
        return *this;
    }

    LogStream& operator<<(char val)
    {
        buffer_.append(&val , 1);//bool类型通过1或者0表示
        return *this;
    }
    LogStream& operator<<(const char *val)
    {
        if(val != NULL)//防御性编程
            buffer_.append(val , strlen(val));//bool类型通过1或者0表示
        else
            buffer_.append("(null)" , 6);
        return *this;
    }

    LogStream& operator<<(const unsigned char *val)
    {
        *this << reinterpret_cast<const char *>(val);//????为什么是这样的
        return *this;
    }


    LogStream& operator<<(float val)
    {
        *this << static_cast<double>(val);
        return *this;
    }
    LogStream& operator<<(double val)
    {
        if (buffer_.avail() >= kMaxNumericSize){
            int len = snprintf(buffer_.current() , kMaxNumericSize , "%.12g" , val);
            buffer_.add(len);
        }
        return *this;
    }
    LogStream& operator<<(long double val)
    {
        if (buffer_.avail() >= kMaxNumericSize){
            int len = snprintf(buffer_.current() , kMaxNumericSize , "%.12Lg" , val);
            buffer_.add(len);
        }
        return *this;
    }
    LogStream& operator<<(const std::string &val)//通过临时对象重载string类型,临时对象属于右值,不允许修改其内容,因此必须const
    {
        buffer_.append(val.c_str() , val.length());//string类型
        return *this;
    }
private:
    LogStream(LogStream &rth);//禁止拷贝
    LogStream &operator=(LogStream &rth);//禁止赋值

    const int kMaxNumericSize = 32;

    template<typename T>
    size_t convert(char buf[], T value)//模板转换函数,整数转换成字符串
    {
        T i = value;
        char *p = buf;
        const char digits[] = "9876543210123456789";
        const char *zero = digits + 9;
        do
        {
            int lsd = static_cast<int>(i % 10);//取出最低位
            i /= 10;
            *p++ = zero[lsd];//直接将字符串赋值进去
        } while (i != 0);

        if (value < 0)
        {
            *p++ = '-';
        }
        *p = '\0';
        std::reverse(buf, p);//反转一下
        return p - buf;//返回长度
    }

    template<typename T>
    void formatInteger(T v)//私有的模板函数,将各种数字类型,转换成对应的字符串类型,便于写入Log
    {
        // buffer容不下kMaxNumericSize个字符的话会被直接丢弃
        if (buffer_.avail() >= kMaxNumericSize)
        {
            size_t len = convert(buffer_.current(), v);//直接格式化到buffer_中
            buffer_.add(len);//缓冲区+len
        }
    }
    Buffer buffer_;//缓冲区
};

#endif // LOGSTREAM_H
