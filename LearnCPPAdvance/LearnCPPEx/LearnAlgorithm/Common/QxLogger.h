#pragma once
#include <cstdio>
#include <functional>


// 用来辅助打印一些内容
class QxLogger
{
public:
    static void Print(int);
    static void Print(float);
    static void Print(double);
    static void Print(char);
    // static void Print()

    
    template<typename  T>
    static void Print(T& );

    template<typename T>
    static void CallPrint(T element);

    template<typename  T>
    static void Print(const T* s )
    {
        s ? Print(s) : printf("<Null>");
    }
};

 //template<typename T>
 //static void MyPrint(T x)
 //{
 //    QxLogger::Print(x);
 //}

template<typename T>
static void MyPrint(T* x)
{
    x ? MyPrint(*x) : printf("<Null>");
}

template<typename T>
static void MyPrint(T& x)
{
    QxLogger::Print(x);
}

template<typename T>
static void MyPrint(const T& x)
{
    QxLogger::Print(x);
}

static void MyPrint(char* x)
{
    printf("%s", x ? x : "<Null>");
}

template<>
static void MyPrint(const char* x)
{
    printf("%s", x ? x : "<Null>");
}

template<typename F>
struct PrintOperator
{
    void operator()(F& e)
    {
        MyPrint(e);
    }
};

template <typename ElementType>
void QxLogger::Print(ElementType& s)
{
	printf("%s[%d]", typeid(s).name(), s.GetSize());
	// auto test =  MyPrint<charPtr>;
	//std::function<void(T::ValueType)> functor = MyPrint<T::ValueType>;
    // std::function<void(int)> functor = MyPrint<int>;
	// auto test = MyPrint<T::ValueType>;
    PrintOperator<ElementType::ValueType> functor;
	s.Traverse(functor);
	printf("\n");
}


template <typename T>
void QxLogger::CallPrint(T element)
{
    MyPrint(element);
}


