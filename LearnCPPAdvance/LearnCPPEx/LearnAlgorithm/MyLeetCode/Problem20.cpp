
#include <stack>
#include <string>
#include <unordered_map>

namespace 
{
class Solution {
public:
    bool isValid(std::string s) {

        std::unordered_map<char, char> validPairMap =
            {
                {')', '('},
                {'}', '{'},
                {']', '['}
            };

        std::stack<char> verifyStack;
        
        for (char letter : s)
        {
            // 是否是后半括号
            if (validPairMap.find(letter) != validPairMap.end())
            {
                // 验证当前letter和栈顶是否匹配
                if (verifyStack.empty() || 
                    validPairMap[letter] != verifyStack.top()) // 不匹配false
                {
                    return false;
                }
                else // 匹配，顶部出栈
                {
                    verifyStack.pop();
                }
            }
            else
            {
                // 入栈
                verifyStack.push(letter);
            }
        }

        return verifyStack.empty();
    }
};
}
