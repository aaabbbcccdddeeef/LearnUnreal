#include <stdio.h>
#include <windows.h>

// 这个配置下，下面的结构占用 6
//#pragma pack(1)

// 默认是pack4 这种情况下，下面输出12
//#pragma pack(4)

struct{
    char c1;  
    int i;    
    char c2;  
}x2;

//int main()
//{
//    printf("%d\n",sizeof(x2));  // 输出和上面的pack配置相关
//    system("pause");
//    return 0;
//}

#include <iostream>
using namespace std;

#define CACHE_LINE   32
#define CACHE_ALIGN __declspec(align(CACHE_LINE))

namespace 
{
// #pragma pack(8)

struct CACHE_ALIGN S1  
{
    int a, b, c, d;
};

struct S3
{
    struct S1 s1;                   
    int a;        
};
// #pragma pack()

struct Test
{
    char m = 0;
    CACHE_ALIGN int n = 0;
};

__declspec(align(16))
struct Test2
{
    char m = 0;
};


struct Test3
{
    char m = 0;
    double x ;
    // Test2 n;
    float n ;
};

struct Test4
{
    char n;
    Test3 x;
};


    // 继承这个类来实现内存对齐的验证
    template<class DerivedType>    
    struct AlignmentType
    {
        static void* operator new(size_t size)
        {
            const size_t alignedSize = __alignof(DerivedType);

            static_assert(alignedSize > 8,
                "Aligned new is only useful for type with >8 byte alignment.  Did you forget a __declspec(align) on DerivedType?");

            void* ptr = _aligned_malloc(size, alignedSize);

            if (!ptr)
            {
                throw std::bad_alloc();
            }
            return ptr;
        }

        static void operator delete(void* ptr)
        {
            _aligned_free(ptr);
        }
    };

    __declspec(align(16))
    class TestClass1  : public AlignmentType<TestClass1>
    {
    public:
            char x;
    };
}


void TestAlignment()
{
    CACHE_ALIGN int i = 2;

    // Test j;
    // // cout << sizeof(j.n) << endl;
    // // cout << sizeof(j) << endl;
    // // cout << alignof(Test) << endl;
    //
    // Test2 w;
    // // cout << sizeof(w) << endl;
    // // cout << alignof(Test2) << endl;
    //
    // Test4 x;
    // cout << sizeof(x) << endl;
    // cout << alignof(Test4) << endl;
    //
    // cout << sizeof(S1) << endl;
    // cout << sizeof(S3) << endl;

    // TestClass1 a;
    // a.x = 1;
    //
    // cout << a.x << endl;

    TestClass1* test = new TestClass1();
    test->x = 100;
    
    cout << test->x << endl;

    delete test;
}

// int main()
// {
//     TestAlignment();
//
//     return 0;
// }