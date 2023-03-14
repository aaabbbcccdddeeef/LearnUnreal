
#include <stack>

namespace 
{


// Definition for singly-linked list.
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

    class Solution {
    public:
        // 回文的特点是从前往后和从后往前一样，
        bool isPalindrome(ListNode* head) {

            ListNode* nodePtr = head;
            std::stack<int>  tmpStack;
            while (nodePtr)
            {
                tmpStack.push(nodePtr->val);
                nodePtr = nodePtr->next;
            }

            // 重置为head，从头遍历和stack pop比较
            nodePtr = head;
            while (nodePtr)
            {
                int stackTop = tmpStack.top();
                if (nodePtr->val != stackTop)
                {
                    return false;
                }
                tmpStack.pop();

                nodePtr = nodePtr->next;
            }
            return  true;
        }
    };
}
