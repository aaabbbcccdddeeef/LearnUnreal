#include <vector>

class Solution {
public:
    // 贪心算法实现，意思是只要当前一天是涨的就累积，否则不累加就是最大利润
    int maxProfit(std::vector<int>& prices) {
        int total = 0;

        for (int i = 0; i < prices.size() - 1; ++i)
        {
            total += std::max(prices[i + 1] - prices[i], 0);
        }
        return total;
    }
};
