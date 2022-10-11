#include <stdio.h>
#include <windows.h>

// 这个配置下，下面的结构占用 6
//#pragma pack(1)

// 默认是pack4 这种情况下，下面输出12
//#pragma pack(4)

struct{
    char c1;  
    int i;    
    char c2;  
}x2;

int main()
{
    printf("%d\n",sizeof(x2));  // 输出和上面的pack配置相关
    system("pause");
    return 0;
}