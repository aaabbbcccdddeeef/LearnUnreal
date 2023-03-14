#include <queue>
#include <vector>

using namespace std;

namespace 
{
    
class Solution {
public:
    void rotate(vector<std::vector<int>>& matrix) {

        std::queue<int> valueQueue;
        for (const auto& row : matrix)
        {
            for (const auto& element : row)
            {
                valueQueue.push(element);
            }
        }

        for (int i = matrix.size() - 1; i >= 0; --i)
        {
            for (int j = 0; j < matrix.size(); ++j)
            {
                matrix[j][i] = valueQueue.front();
                valueQueue.pop();
            }
        }
    }
};
}
