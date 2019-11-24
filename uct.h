#include<cmath>
#include<stack>
using namespace std;
#define C 1 // 系数，通过调整这个改变搜索的深度与广度

class UCTreeNode { // UCT
    private:
        UCTreeNode* ptrChildren[2000];
        bool isLeaf = true;
        double visits = 0;
        double wins = 0; // 获胜次数
        // 关于局面评估
        // score = wins_child / visits_child + C*sqrt(log(visits + 1) / visits_child)
        // 其中wins_child / visits_child代表胜率
        int childNum = 2000;
        // 拓展叶子节点
        void expand() { 
            if(!isLeaf) return;
            for (int i = 0; i < childNum; i++)
                ptrChildren[i] = new UCTreeNode();
            isLeaf=false;
        }
        // 更新数据
        void update(int win) {
            visits++;
            wins += win;
        }
    public:
        UCTreeNode() { // 构造函数
            for (int i = 0; i < childNum; i++) {
                ptrChildren[i] = 0;
            }
        }
        UCTreeNode(const UCTreeNode &tree) { // 拷贝构造函数
            if (isLeaf) return;
            for (int i = 0; i < childNum; i++) {
                ptrChildren[i] = new UCTreeNode(*tree.ptrChildren[i]);
            }
        }
        // 筛选最优子结点
        int selectChild() {
            int ret = 0;
            double bestScore = -786554453;
            for(int i=0; i<childNum; i++) { // 遍历所有子结点
                UCTreeNode* ptrCurChild = ptrChildren[i]; 
                // 局面评估
                double curScore = ptrCurChild->wins / ptrCurChild->visits + C * sqrt(log(visits + 1) / ptrCurChild->wins); 
                if(curScore >= bestScore) {
                    ret = i;
                    bestScore = curScore;
                }
            }
            return ret;
        }
        void iterate() { // 遍历
            stack<UCTreeNode*> visited;
            UCTreeNode* ptrCur = this;
            visited.push(this);
            int bestChild = 0; // 选取最优的子节点
            while (!ptrCur->isLeaf) {
                bestChild = ptrCur->selectChild();
                ptrCur = ptrCur->ptrChildren[bestChild];
                visited.push(ptrCur);
            } // 跳出这里时，ptrCur已经是叶子节点
            ptrCur->expand();
            bestChild = ptrCur->selectChild();
            ptrCur = ptrCur->ptrChildren[bestChild];
            visited.push(ptrCur);
            // --------------------------------------------------------------------------------------------------
            double win = 0; // 这里需要判断是否获胜
            while (!visited.empty()) {
                ptrCur = visited.top();
                ptrCur->update(win);   // 依次更新节点数值
                visited.pop();
            }
        }
};