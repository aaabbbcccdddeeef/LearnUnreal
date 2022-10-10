//多线程抢占输出
#include <iostream>
#include <windows.h>

using namespace std;

void ThreadUser() { //线程入口
    cout << "child thread start" << endl;
    for (int i = 0; i < 10; i++) { //抢占循环
        cout << "child thread {" << i << "} get time slice " << endl; //输出信息
        Sleep(100); //抢占延时
    }
    cout << "child thread end" << endl;
}

static void Test1()
{
    cout << "child thread start" << endl;
    HANDLE h; //线程句柄
    h=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadUser, NULL, CREATE_SUSPENDED, 0); //创建子线程
    ResumeThread(h);  //启动子线程
    for (int i = 0; i < 10; i++) { //抢占循环
        cout << "main thread {" << i << "}get time slice" << endl; //输出信息
        Sleep(100); //抢占延时
    }
    Sleep(1000); //等待子线程
    CloseHandle(h); 
    cout << "child thread end" << endl;
    system("pause");
}

int main() {
    Test1();
    return 0;
}