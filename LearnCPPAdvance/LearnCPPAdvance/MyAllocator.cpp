#include "MyAllocator.h"


namespace 
{
    class TestClass
    {
    public:
        TestClass()
        {
            a = b = 10;
            c = 0.f;
            d = 0;
            test = nullptr;

            std::cout << "TestClass constructor1 is invoked" << std::endl;
        }

        TestClass(int inA, int inB, float inC)
        {
            a = inA;
            b = inB;
            c = inC;
            test = nullptr;
            std::cout << "TestClass constructor2 is invoked" << std::endl;
        }
        
        ~TestClass()
        {
            a = 0;
            std::cout << "TestClass destructor is invoked" << std::endl;
        }

    public:
        static void* operator new(std::size_t sz)
        {
            std::printf("TestClass operator new is called\n");
            if (sz == 0)
                ++sz; //avoid std::malloc(0) which may return nullptr on success

            if (void* ptr = std::malloc(sz))
            {
                return ptr;
            }
            throw std::bad_alloc{}; 
        }

        static void operator delete(void* ptr) noexcept
        {
            std::puts("TestClass delete is called");
            std::free(ptr);
        }

        
        int a;
        int b;
        float c;
        char d;
        char* test;
    };
}

void TestStdAllocator()
{
    std::allocator<TestClass> testAllocator;
    using TestTraits = std::allocator_traits<std::allocator<TestClass>>;
    
    // TestClass* testA = testAllocator.allocate(1);
    constexpr  size_t testNum = 1;
    TestClass* testA = TestTraits::allocate(testAllocator, testNum);
    TestTraits::construct(testAllocator, testA);
    TestTraits::construct(testAllocator, testA, 1, 2, 3);
    
    // testAllocator.construct(testA);
    
    
    // testAllocator.destroy(testA);
    TestTraits::destroy(testAllocator, testA);
    // testAllocator.deallocate(testA, sizeof(TestClass));
    TestTraits::deallocate(testAllocator, testA, sizeof(TestClass));
}