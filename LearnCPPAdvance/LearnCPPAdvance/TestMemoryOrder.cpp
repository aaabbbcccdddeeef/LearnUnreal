#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
// 顺序一致
std::atomic<bool> x, y;
std::atomic<int> z;
int w;

// 这个测试中用 获取-发布模型release/acquire，理论上来说z可能是0，但很难复现，
void write_x()
{
    x.store(true, std::memory_order_release);
}
void write_y()
{
    y.store(true, std::memory_order_release);
}
void read_x_then_y()
{
    while (!x.load(std::memory_order_acquire));
    if (y.load(std::memory_order_acquire))
        ++z;
    //while (!y.load(std::memory_order_seq_cst))
    //{
    //}
    ////++z;
    //++w;
}
void read_y_then_x()
{
    while (!y.load(std::memory_order_acquire));
    if (x.load(std::memory_order_acquire))
        ++z;
    //while (!x.load(std::memory_order_seq_cst))
    //{
    //}
    ////++z;
    //++w;
}

static void Test()
{
    x = false;
    y = false;
    z = 0;
    w = 0;
    std::thread a(write_x);
    std::thread b(write_y);
    std::thread c(read_x_then_y);
    std::thread d(read_y_then_x);
    a.join();
    b.join();
    c.join();
    d.join();
    assert(z.load() != 0);

    using namespace std;
    cout << "final z = " << z << endl;
    //cout << "final w = " << w << endl;
}
//int main()
//{
//   
//    for (int i = 0; i < 10000; ++i)
//    {
//        Test();
//    }
//}