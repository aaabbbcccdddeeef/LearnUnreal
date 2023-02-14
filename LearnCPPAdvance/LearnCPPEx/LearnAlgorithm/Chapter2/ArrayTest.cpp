
#include <vector>

namespace 
{
    class MyClass
    {
    public:
        MyClass(int inA)
        {
            a = inA;
        }
    // private:
        MyClass(const MyClass&)
        {
            
        }
        MyClass& operator=(const MyClass&)
        {
            
        }
        int a = 0;
    };
}

void TestVector()
{
    using namespace std;
    vector<MyClass> test;
    test.emplace_back(1);
    
}
