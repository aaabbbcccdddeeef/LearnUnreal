#include <iostream>
#include <stdio.h>
#include <windows.h>

namespace 
{
    class Parent
    {
    public:
        Parent()
        {
        }

        Parent(char inA, float inB)
            : a(inA), b(inB)
        {
        }

        virtual void Test1()
        {
            
        }
        char a = 0;
        float b = 0;
    };

    class Child1 : public Parent
    {
    public:
        Child1()
            : Parent(1, 1)
        {
        }

        int test1 = 1;
    };

    class Child2 : public Parent
    {
    public:
        Child2()
            : Parent(1, 1)
        {
        }

        int test2 = 1;
    };
    
}

// int main()
// {
//     Parent a1;
//     Parent* p = &a1;
//     Child1* pC1 = static_cast<Child1*>(p);
//     Child1* pC1_Dynamic = dynamic_cast<Child1*>(p);
//     std::cout << pC1->test1 << std::endl;
//
//     std::cout << pC1_Dynamic->test1 << std::endl;
//     
//
//     return 0;
// }