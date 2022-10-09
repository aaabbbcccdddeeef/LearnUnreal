#include <iostream>
#include <thread>


void Hello()
{
    std::cout << "Hello Concurrent world\n";
}


int main(int argc, char* argv[])
{
    std::thread t(Hello);
    t.join();
    if (t.joinable())
    {
        t.join();
    }
   
    return 0;
}
