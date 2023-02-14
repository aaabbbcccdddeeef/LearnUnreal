
#include <algorithm>
#include <unordered_map>
#include <vector>


namespace 
{
    using namespace std;
    class Solution
    {
    public:
       int maxSubArray(vector<int>& nums)
       {
           if (nums.empty())
           {
               return 0;
           }
           int curSum = nums[0];
           int maxSum = curSum;

           int frontPtr = 0;
           int backPtr = frontPtr + 1;

           // 前一个数值是否正
           bool bIsPrevPositive = false;
           while (backPtr < nums.size())
           {
               if (nums[backPtr] > maxSum) // 先处理max sum是负数的情况
               {
                   maxSum = nums[backPtr];
                   frontPtr = backPtr;
                   backPtr = frontPtr + 1;

                   curSum = maxSum;
                   continue;
               }
               if (nums[backPtr] > 0 ) //
               {
                   if (bIsPrevPositive)
                   {
                       curSum += nums[backPtr];
                       backPtr++;
                   }
                   else
                   {
                       bIsPrevPositive = true;
                       curSum = nums[backPtr];
                       frontPtr = backPtr;
                       backPtr = frontPtr + 1;
                   }
               }
               else
               {
                   backPtr++;
               }
               if (curSum > maxSum)
               {
                   maxSum = curSum;
               }
           }
           return  curSum;
       }

       int maxSubArray2(vector<int>& nums)
       {
           int ans = nums[0], previous = nums[0];
           for(int i = 1; i < nums.size(); i++){
               previous = max(previous+nums[i], nums[i]);
               ans = max(previous, ans);
           }
           return ans;
       }
    };
}
