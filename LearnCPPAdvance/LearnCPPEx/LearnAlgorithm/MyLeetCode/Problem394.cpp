#include <stack>
#include <string>

namespace 
{
using namespace std;
    
class Solution {
private:
public:
    // 遍历处理字符串
    // 3种字符类型，数字组成一个完成数值进数字栈，'['进字母栈、']'进行处理
    string decodeString(std::string s) {
        stack<int> numStack;
        stack<string> strStack;

        int tmpNum = 0;
        string str;
        for (size_t i = 0; i < s.size() && s[i] != '\0'; ++i)
        {
            char curLetter = s[i];

            if (isdigit(curLetter))
            {
                tmpNum = tmpNum * 10 + curLetter - '0';
            }
            else if (isalpha(curLetter))
            {
                str += curLetter;
            }
            else if (curLetter == '[') // 遇到'['时，将当前num/str都进栈，并clear
            {
                numStack.push(tmpNum); 
                tmpNum = 0;

                
                strStack.push(str);
                str = ""; // 开始记录新的字符串
            }
            else // 这里相当于 是']'，假设输入字符串只有3中字母
            {
                int curRepeatNum = numStack.top();
                numStack.pop();

                string tmpExpandStr = "";
                for (int i = 0; i < curRepeatNum; ++i)
                {
                    tmpExpandStr += str;
                }
                
                // str = tmpExpandStr;
                str = strStack.top() + tmpExpandStr;
                strStack.pop();
            }
        }

        return str;
    }
};
}

