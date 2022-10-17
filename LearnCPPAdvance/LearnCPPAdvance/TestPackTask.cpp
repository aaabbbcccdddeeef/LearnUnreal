#include <iostream>
#include <cmath>
#include <thread>
#include <future>
#include <functional>

using namespace std;

// unique function to avoid disambiguating the std::pow overload set
static int f1(int x, int y) 
{
    cout << "task_bind theadID:  " << std::this_thread::get_id() << endl;
    return std::pow(x,y);
}

static int f2(int x, int y)
{
	cout << "task_thread theadID:  " << std::this_thread::get_id() << endl;
	return std::pow(x, y);
}
 
void task_lambda()
{
    std::packaged_task<int(int,int)> task([](int a, int b) {
        cout << "task_lambda theadID:  " << std::this_thread::get_id() << endl;
        return std::pow(a, b); 
    });
    std::future<int> result = task.get_future();
 
    task(2, 9);
 
    std::cout << "task_lambda:\t" << result.get() << '\n';
}
 
void task_bind()
{
    std::packaged_task<int()> task(std::bind(f1, 2, 11));
    std::future<int> result = task.get_future();
 
    task();
 
    std::cout << "task_bind:\t" << result.get() << '\n';
}
 
void task_thread()
{
    std::packaged_task<int(int,int)> task(f2);
    std::future<int> result = task.get_future();
 
    std::thread task_td(std::move(task), 2, 10);
    task_td.join();
 
    std::cout << "task_thread:\t" << result.get() << '\n';
}
 
int main()
{
    cout << "main theadID:  " << std::this_thread::get_id() << endl;
    task_lambda();
    task_bind();
    task_thread();
    system("pause");
}