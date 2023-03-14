#include <vector>
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

    void inorderTraversal(TreeNode* inRoot, vector<int>& outRes)
    {
        if (!inRoot)
        {
            return;
        }

        inorderTraversal(inRoot->left, outRes);
        outRes.push_back(inRoot->val);
        inorderTraversal(inRoot->right, outRes);
    }
    
public:
    // 中序遍历
    vector<int> inorderTraversal(TreeNode* root) {

        vector<int> res;
        
        inorderTraversal(root, res);

        return move(res);
    }
};