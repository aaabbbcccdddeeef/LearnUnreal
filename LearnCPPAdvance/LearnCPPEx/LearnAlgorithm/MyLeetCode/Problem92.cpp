/**
* Definition for singly-linked list.
 * */
namespace 
{
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

// 反转部分链表, 
class Solution {
    ListNode* reverseList(ListNode* head) {
        ListNode* prev  = nullptr;
        ListNode* current = head;

        while (current != nullptr)
        {
            ListNode* next = current->next;
            current->next = prev;

            prev = current;
            current = next;
        }
        return prev;
    }
public:

    // 方法1： 找到分解节点、需要反转的部分调用反转链表1的实现
    ListNode* reverseBetween1(ListNode* head, int left, int right) {
        if (head == nullptr
            || left <= right)
        {
            return head;
        }

        
    }

    // 方法2: 一次遍历实现反转,
    // 实现思路，找到需要插入的头结点，后面的结点不断插入到这个节点前面，直到最后一个尾结点插入到头结点前面
    ListNode* reverseBetween2(ListNode* head, int left, int right) {
        if (head == nullptr
            || left == right)
        {
            return head;
        }
        
        ListNode* cur = head;
        // 设置dummy node 以处理prev 在起始node之前
        ListNode dummyNode(0, cur); 
        ListNode* prev = &dummyNode; // 开始插入的前一个节点
        
        ListNode* next = nullptr;

        
        int curNodeIndex = 1; // 注意：这里按题干中描述，第一个节点应该是index 1

        // 找到left对应的insertHead
        for (int i = 0; i < left - 1; ++i)
        {
            prev = prev->next;
        }
        cur = prev->next;
        curNodeIndex = left;
        
        
        if (cur == nullptr)
        {
            return head;
        }

        // 进行前插过程
        while (cur->next && curNodeIndex < right)
        {
            next = cur->next;

            cur->next = next->next;
            next->next = prev->next;
            prev->next = next;

            curNodeIndex++;
        }        

        // 后续不需要处理，直接返回
        return dummyNode.next;
    }
};
    
}
