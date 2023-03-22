
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

namespace 
{
    class TestA;
    
    std::ostream& operator<<(std::ostream& os, const TestA& inA);
    class TestA
    {
    public:
        TestA(int inA, float inB)
            : a(inA), b(inB)
        {
        }

        TestA(const TestA& other)
            : a(other.a), b(other.b)
        {
            std::cout << "Copy ctor is called" << std::endl;
        }

        TestA& operator==(const TestA& other)
        {
            a = other.a;
            b = other.b;
            std::cout << "Copy assign ment is called " << std::endl;
        }

        // bool operator<(const TestA& other) const
        // {
        //     return a < other.a;
        // }

        std::string ToString() const
        {
            std::stringstream tmpStrStream;
            tmpStrStream << "{ " << a << "," << b << "}";
            return  tmpStrStream.str();
        }

        friend  std::ostream& operator<<(std::ostream& os, const TestA& inA);

    private:

    public:
        int a;
        float b;
    };

    std::ostream& operator<<(std::ostream& os, const TestA& inA) 
    {
        os <<  inA.ToString();
        return os;
    }

    template<typename T>
    void PrintVector(const std::vector<T>& InVector)
    {
        std::cout << "{ ";
        for (const auto& element : InVector)
        {
            std::cout << element;
        }
        std::cout << "}" << std::endl;
    }
}

void TestStdVector1()
{
    using namespace std;
    vector<TestA> testArray1;
    testArray1.reserve(3);
    TestA testA(1, 2);
    testArray1.push_back(testA);
    testArray1.push_back(testA);
    testArray1.push_back(testA);

    std::cout << "============" << std::endl;
    // testArray1.resize(1);
    testArray1.push_back(testA);
    
    PrintVector(testArray1);

    std::sort(testArray1.begin(), testArray1.end(),
        [](const TestA& left, const TestA& right)
        {
            return left.a < right.a;
        }
        );
}