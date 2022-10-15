

#include <iostream>
#include <mutex>
// static void 
//
// void main()
// {
//     
// }
std::mutex TestLock1;
std::mutex TestLock2;


int TestCounter;
int TestMax = 1000;

using namespace  std;

void TestThreadFunc()
{
    while (true)
    {
        std::lock_guard<std::mutex> lock(TestLock2);
        if (TestCounter < TestMax)
        {
            TestCounter++;
        }
        else
        {
            break;
        }
    }
}

static void Test()
{
    std::thread thread1(TestThreadFunc);
    std::thread thread2(TestThreadFunc);

    thread1.join();
    thread2.join();

    std::cout << "TestCounter = " << TestCounter << endl;
}

void main()
{
    Test();
}