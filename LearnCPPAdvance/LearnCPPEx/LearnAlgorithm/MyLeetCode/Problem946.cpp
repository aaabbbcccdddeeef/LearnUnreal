
#include <stack>
#include <vector>
using namespace std;
class Solution {
public:
    bool validateStackSequences(vector<int>& pushed, vector<int>& popped) {

        stack<int> valueStack;
        for (int pushIndex = 0, popIndex = 0; pushIndex < pushed.size() && popIndex < popped.size(); pushIndex++)
        {
            valueStack.emplace(pushed[pushIndex]);

            while (!valueStack.empty() &&
                (valueStack.top() ==  popped[popIndex]))
            {
                valueStack.pop();

                popIndex++;
            }
        }
        return valueStack.empty();
    }
};