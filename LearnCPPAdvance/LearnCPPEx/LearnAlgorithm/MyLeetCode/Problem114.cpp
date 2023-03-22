
#include <stack>
#include <vector>

namespace 
{
    
/**
 * Definition for a binary tree node.
 * */
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

using namespace std;
class Solution {

    void PreTraverse(TreeNode* inRoot, vector<TreeNode*>& outNodeList)
    {
        if (inRoot != nullptr)
        {
            outNodeList.push_back(inRoot);
            PreTraverse(inRoot->left, outNodeList);
            PreTraverse(inRoot->right, outNodeList);
        }
    }
    
public:
    
    void flatten(TreeNode* root) {

        std::vector<TreeNode*> nodeList;

        PreTraverse(root, nodeList);

        for (int i = 1; i < nodeList.size(); ++i)
        {
            TreeNode* pre = nodeList[i - 1];
            TreeNode* cur = nodeList[i];
            pre->left = nullptr;
            pre->right = cur;
        }
    }

    // 迭代实现
    void flatten2(TreeNode* root)
    {
        vector<TreeNode*> nodeList;
        stack<TreeNode*> nodeStack;
        TreeNode* curNode = root;
        while (curNode != nullptr || !nodeStack.empty())
        {
            while (curNode != nullptr)
            {
                nodeList.push_back(curNode);
                nodeStack.push(curNode);
                curNode = curNode->left;
            }
            curNode = nodeStack.top();
            nodeStack.pop();

            curNode = curNode->right;
        }


        for (int i = 1; i < nodeList.size(); ++i)
        {
            TreeNode* pre = nodeList[i - 1];
            TreeNode* cur = nodeList[i];
            pre->left = nullptr;
            pre->right = cur;
        }
    }
};
}
