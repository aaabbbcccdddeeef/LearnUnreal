#include <iostream>
#include <string>
#include <chrono>
#include <string_view>
 
//计时器
class Timer
{
private:
    std::string title;
    std::chrono::high_resolution_clock::time_point m_start, m_stop;
public:
    Timer(const std::string& title): title(title)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }
 
    ~Timer()
    {
        stop();
    }
 
    void stop()
    {
        m_stop = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_stop - m_start);
        std::cout << title << " " << (ms.count()) * 0.001 << "s\n";
    }
};
 
void FunctionWithString(const std::string& string)
{
}
 
void FunctionWithString(const std::string_view& stringView)
{
}
 
int main()
{
 
    {
        Timer timer("std::string");
        for (int i = 0; i < 1000000; i++)
        {
            std::string name = "Yang Xunwu";
            std::string firstName = name.substr(0, 4);
            std::string lastName = name.substr(4, 9);
            FunctionWithString(firstName);
            FunctionWithString(lastName);
        }
    }
 
    {
        Timer timer("std::string_view");
        for (int i = 0; i < 1000000; i++)
        {
            const char* name = "Yang Xunwu";
            std::string_view firstName = std::string_view(name, 4);
            std::string_view lastName = std::string_view(name + 4, 9);
            FunctionWithString(firstName);
            FunctionWithString(lastName);
        }
    }
}