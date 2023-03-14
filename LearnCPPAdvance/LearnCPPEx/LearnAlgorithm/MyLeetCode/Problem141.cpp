

#include <unordered_set>
/**
 * Definition for singly-linked list.
 * */
struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(nullptr) {}
};

// 判断是否环形链表
class Solution {
public:
    // 方法1： 用hash set
    bool hasCycle(ListNode *head) {
        std::unordered_set<ListNode*> hashSet;

        ListNode* curNode = head;
        while (curNode)
        {
            if (hashSet.count(curNode))
            {
                return true;
            }
            hashSet.emplace(curNode);

            curNode = curNode->next;
        }
        return false;
    }

    // 方法2： 快慢双指针
    bool hasCycle2(ListNode *head) {
        if (head == nullptr || head->next == nullptr)
        {
            return false;
        }
        
        ListNode* slowPtr = head;
        ListNode* fastPtr = head->next;

        while (slowPtr != fastPtr)
        {
            if (fastPtr == nullptr || fastPtr->next == nullptr)
            {
                return false;
            }

            slowPtr = slowPtr->next;
            fastPtr = fastPtr->next->next;
        }
        return true;
    }
};