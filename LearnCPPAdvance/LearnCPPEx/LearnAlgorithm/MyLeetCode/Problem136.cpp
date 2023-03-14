
#include <vector>

namespace 
{

    using namespace std;

    // 只出现一次的数字，
    class Solution {
    public:

        // 这题的关键是用异或运算的性质
        int singleNumber(std::vector<int>& nums) {
            int res = 0;
            for (int element : nums)
            {
                res ^= element;
            }
            return res;
        }
    };
}
