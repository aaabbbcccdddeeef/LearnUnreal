#include <cstddef>
#include <functional>
#include <vector>
/*
 * */
// Definition for a QuadTree node.
class Node {
public:
    bool val;
    bool isLeaf;
    Node* topLeft;
    Node* topRight;
    Node* bottomLeft;
    Node* bottomRight;
    
    Node() {
        val = false;
        isLeaf = false;
        topLeft = nullptr;
        topRight = nullptr;
        bottomLeft = nullptr;
        bottomRight = nullptr;
    }
    
    Node(bool _val, bool _isLeaf) {
        val = _val;
        isLeaf = _isLeaf;
        topLeft = NULL;
        topRight = NULL;
        bottomLeft = NULL;
        bottomRight = NULL;
    }
    
    Node(bool _val, bool _isLeaf, Node* _topLeft, Node* _topRight, Node* _bottomLeft, Node* _bottomRight) {
        val = _val;
        isLeaf = _isLeaf;
        topLeft = _topLeft;
        topRight = _topRight;
        bottomLeft = _bottomLeft;
        bottomRight = _bottomRight;
    }
};

using namespace std;

class Solution {
public:
    Node* construct(vector<vector<int>>& grid) {

        function<Node*(int, int, int, int)> dfs = [&](int left, int top,  int right, int bottom)
        {
            for (int i = top; i < bottom; ++i)
            {
                for (int j = left; j < right; ++j)
                {
                    if (grid[i][j] != grid[top][left]) // 不是叶节点
                    {
                        return new Node(
                            true,
                            false,
                            dfs(left, top, (left + right)/2, (top + bottom)/2),
                            dfs((left+right)/2, top, right, (top+bottom)/2),
                            dfs(left, (top+bottom)/2, (left+right)/2, bottom),
                            dfs((left+right)/2, (top+bottom)/2, right, bottom)
                            );
                    }
                }
            }
            // 是叶节点
            return new Node(grid[top][left], true, nullptr, nullptr, nullptr, nullptr);
        };

        return   dfs(0, 0, grid.size(), grid.size());
    }
};