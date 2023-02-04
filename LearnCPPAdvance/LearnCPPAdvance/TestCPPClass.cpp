
#include <iostream>

namespace 
{
    using namespace std;
    class TestClassA
    {
    public:

        void Test1()
        {
            std::cout << "TestClassA::Test1" << std::endl;
            cout << "Address = " << (void*)this << endl;
            // cout << "testData1 = " << this->testData1 <<endl;
        }

        void Test2()
        {
            std::cout << "TestClassA::Test2" << std::endl;
            cout << "Address = " << (void*)this << endl;
            // cout << "testData1 = " << this->testData1 <<endl;
            Test1();
        }

    private:
        int testData1  = 234;
    };
}


void TestCPPClass()
{
    // 下面这段测试中
    // 1. 语法一定正确
    // 2. 执行未必会报exception, 只要Test1/Test2函数内部没有访问非静态的数据对象就可以正确执行
    // 实践中一般不会这样写，但底层的一部分代码确实有这样用的，例如参考UE4源码FLinearAllocatorPolicy::ForElementType::ResizeAllocation
    TestClassA* test = nullptr;
    test->Test1();
    test->Test2();
}