#include<cstdio>
#include<iostream>
#include<algorithm>
#include<cmath>
#include<vector>
#include<stack>
#include<queue>
using namespace std;
#define C 1.14514 // 系数，通过调整这个改变搜索的深度与广度
#define EPS 1e-7

// 局面评估函数待优化！

class ChessBoard { // 每个棋盘都是UCTree的一个节点！（暴论）
    private:
        int board[8][8]; // 棋盘本盘
        //           → ↘  ↓  ↙ ←  ↖   ↑   ↗
        int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
        int dy[8] = {1, 1, 0, -1, -1, -1, 0, 1};
        int chess[3][4]; // 存取四个棋所在的xy坐标，1为黑棋，2为白棋,十位数为x，个位数为y
        bool hinted = false;
        int last_move = 0; // 上一次移动
    public:
        // 标记
        short BLACK = 1; // 黑棋
        short WHITE = 2; // 白棋
        short EMPTY = 0;
        short BLOCK = -1; // 障碍物
        short CANGO = 6; // 可以下的点（而且可以进一步放障碍物）
        int turn_player; // 本回合玩家
        int win = 0; // 当前棋盘是否获胜，1为胜，否则为败
        struct SL { // 存放可行解用
            int solution[2000];
            int idx = 0; // 记得每次用完时要归零
        } SolutionList;
        // UCT部分
        ChessBoard* ptrChildren[2000];
        bool isLeaf = true;
        double visits = 0;
        double score = 0; // 考虑使用score来替代wins
        // 关于局面评估
        // judgescore = score_child / visits_child + C*sqrt(log(visits + 1) / visits_child)
        // 其中wins_child / visits_child代表胜率
        int childNum = 0;
        int uct_turnplayer;

        void Initialize();
        void Reset();
        bool In_Board(int x, int y);
        bool Can_Move(int x, int y);
        int Move(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block);
        bool Judge_Win();
        void Next_Turn();
        // 用来寻找所有可以落子和放障碍物的点 面向AI寻找可行解使用
        // 其中SolutionList存放的每个Solution均为一个整数，每位数依次为 起始点x、y，终点x、y，障碍点x、y
        inline void Find_Possible_Move(int xy);
        inline void Find_Possible_Block(int xy_start, int xy_final);
        inline void Find_Solutions();
        // UCT(MCTS) section!
        ChessBoard() { // 构造函数
            for (int i = 0; i < childNum; i++) {
                ptrChildren[i] = 0;
            }
        }
        void copy(const ChessBoard &temp);
        void expand(); // 拓展叶子节点
        void update(int val);
        int selectChild(); // 筛选最优子节点
        void iterate(); // 遍历博弈树
        int getBestSol(); // 返回最好的solution，格式同上
        // 评估函数，计算公式 score = k1*t1 + k2*t2 + k3*p1 + k4*p2 + k5*m
        // 其中t1、t2代表双方的territory值（通过kingMove和queenMove来判断）
        // p1、p2代表位置position的特征值，判断走法相对空格的优劣
        // m是灵活性的评估值
        // k1、k2、k3、k4、k5为参数
        int judgeScore();
        void kingMove(int color, int kingdist[8][8]); // 评估函数用
        void queenMove(int color, int queendist[8][8]);
        int firstbonus = 0.2; // 先手优势
        int turns = 0; // 第几回合
        float k1[3] = {0.14, 0.30, 0.80};
        float k2[3] = {0.37, 0.25, 0.10};
        float k3[3] = {0.13, 0.20, 0.05};
        float k4[3] = {0.13, 0.20, 0.05};
        float k5[3] = {0.20, 0.05, 0.00};
};

void ChessBoard::Initialize() { // initialize
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            board[i][j] = 0; 
    board[0][2] = board[2][0] = board[0][5] = board[2][7] = BLACK;
    board[5][0] = board[7][2] = board[5][7] = board[7][5] = WHITE;
    chess[1][0] = 2; chess[1][1] = 20; chess[1][2] = 5; chess[1][3] = 27;
    chess[2][0] = 50; chess[2][1] = 72; chess[2][2] = 57; chess[2][3] = 75;
    turn_player = BLACK;
}

void ChessBoard::Reset() { 
    ios::sync_with_stdio(false); // iostream加速
    Initialize();
}

bool ChessBoard::In_Board(int x, int y) {
    if(x < 0 || x >= 8) return false;
    if(y < 0 || y >= 8) return false;
    return true;
}

bool ChessBoard::Can_Move(int x, int y) {
    bool ret = false;
    for(int dir=0; dir<8; dir++) {
        int x_next = x + dx[dir];
        int y_next = y + dy[dir];
        if(!In_Board(x_next, y_next)) { // 越界判断
            continue;
        }
        if(!board[x_next][y_next] && board[x_next][y_next] != CANGO) { // 是否被包围
            ret = true;
            break;
        }
    }
    return ret;
}

int ChessBoard::Move(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block) { // 按接口要求，需要转置
    // 错误判断
    if(!In_Board(x_start, y_start) && !In_Board(x_final, y_final) && !In_Board(x_block, y_block)) {
        //cout << "非法坐标：坐标越界！ErrorType:37510\n";
        return 37510;
    }
    if(board[x_start][y_start] != turn_player) {
        //cout << "非法坐标：这个位置没有您的棋！ErrorType:11037\n";
        //cout << "board[" << x_start << "][" << y_start << "] = " << board[x_start][y_start];
        return 11037;
    }
    if((board[x_final][y_final] || board[x_block][y_block]) && !(x_block == x_start && y_block == y_start)) { // 特判回马枪情形
        //cout << "非法坐标：需求坐标已被占用！ErrorType:23333\n";
        return 23333;
    }
    int deltax = abs(x_final - x_start), deltay = abs(y_final - y_start);
    if(!(deltax == 0) && !(deltay == 0) && !(deltax == deltay)) { // 移动合法性判断
        //cout << "非法移动：棋子移动方法不在8个方向上！ErrorType:88888\n";
        return 88888;
    }
    deltax = abs(x_block - x_final), deltay = abs(y_block - y_final);
    if(!(deltax == 0) && !(deltay == 0) && !(deltax == deltay)) { // 障碍物合法性判断
        //cout << "非法障碍：障碍物摆放方法不在8个方向上！ErrorType:10086\n";
        return 88889;
    }
    // 坐 标 移 动
    int chessidx = 0;
    for(;chessidx<4;chessidx++) {if(chess[turn_player][chessidx] == x_start*10 + y_start) break;}
    board[x_start][y_start] = EMPTY; 
    board[x_final][y_final] = turn_player;
    chess[turn_player][chessidx] = x_final*10 + y_final;
    board[x_block][y_block] = BLOCK;
    return 0;
}

bool ChessBoard::Judge_Win() {
    bool locked[3][5]; // 记录对应颜色的第某个棋子是否被锁死
    for(int i=0; i<4; i++) { // 计算其是否被锁死
        locked[BLACK][i+1] = !Can_Move(chess[BLACK][i]/10, chess[BLACK][i]%10);
        locked[WHITE][i+1] = !Can_Move(chess[WHITE][i]/10, chess[WHITE][i]%10);
    }
    locked[BLACK][0] = (locked[BLACK][1] && locked[BLACK][2] && locked[BLACK][3] && locked[BLACK][4]);
    locked[WHITE][0] = (locked[WHITE][1] && locked[WHITE][2] && locked[WHITE][3] && locked[WHITE][4]);
    if(locked[BLACK][0]) {
        if(uct_turnplayer == WHITE) win = 1;
        //Display();
        //cout << "胜者是白方○！\n";
    } else if(locked[WHITE][0]) {
        if(uct_turnplayer == BLACK) win = 1;
        //Display();
        //cout << "胜者是黑方●！\n";
    } else return false;
    return true;
}

void ChessBoard::Next_Turn() {
    turn_player = 3 - turn_player;
    turns++; // 进行了一回合
}

inline void ChessBoard::Find_Possible_Block(int xy_start, int xy_final) {
    // 搜索常用套路
    board[xy_start/10][xy_start%10] = EMPTY;
    board[xy_final/10][xy_final%10] = turn_player;
    for(int dir=0; dir<8; dir++) {
        int x_tmp = xy_final/10 + dx[dir], y_tmp = xy_final%10 + dy[dir]; // 故 技 重 施
        while(In_Board(x_tmp, y_tmp) && board[x_tmp][y_tmp] == EMPTY) { // 在能够落子之后还能放障碍物，那么这就相当于一个Solution
            // 每个Solution均为一个整数，每位数依次为 起始点x、y，终点x、y，障碍点x、y（复读）
            SolutionList.solution[++SolutionList.idx] = (xy_start * 1e4 + xy_final * 1e2 + x_tmp *10 + y_tmp);
            x_tmp += dx[dir], y_tmp += dy[dir];
        }
    }
    board[xy_start/10][xy_start%10] = turn_player;
    board[xy_final/10][xy_final%10] = EMPTY;
}

inline void ChessBoard::Find_Possible_Move(int xy) {
    for(int dir=0; dir<8; dir++) {
        int x_tmp = xy/10 + dx[dir], y_tmp = xy%10 + dy[dir];
        while(In_Board(x_tmp, y_tmp) && board[x_tmp][y_tmp] == EMPTY) { // 能够落子
            Find_Possible_Block(xy, x_tmp*10 + y_tmp);
            x_tmp += dx[dir], y_tmp += dy[dir];
        }
    }
}

inline void ChessBoard::Find_Solutions() {
    SolutionList.idx = 0;
    for(int idx=0; idx<4; idx++) {
        Find_Possible_Move(chess[turn_player][idx]);
    }
}

// UCT section!
void ChessBoard::copy(const ChessBoard &temp){ // 拷贝函数，只考虑必要信息
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++) {
            board[i][j] = temp.board[i][j];
        }
    turn_player = temp.turn_player;
    uct_turnplayer = temp.uct_turnplayer;
    for(int i=1; i<=2; i++)
        for(int j=0; j<4; j++)
            chess[i][j] = temp.chess[i][j];
    turns = temp.turns;
}

void ChessBoard::expand() {
    if(!isLeaf) return;
    if(win) {
        isLeaf = true;
        childNum = 0;
        return;
    }
    Find_Solutions();
    childNum = SolutionList.idx;
    int idx = 0, sol;
    for (int i = 1; i <= SolutionList.idx; i++) { // 生成所有下了可行解的棋盘
        ptrChildren[idx] = new ChessBoard();
        ptrChildren[idx]->copy(*this);
        sol = SolutionList.solution[i];
        while(sol == 0 && i < SolutionList.idx) {
            i++;
            sol = SolutionList.solution[i];
            if(i == SolutionList.idx) {
                break;
            }
        }
        // 模拟下棋
        //cout << sol << endl;
        int ErrorMsg = ptrChildren[idx]->Move(sol/100000, (sol/10000)%10, (sol/1000)%10, (sol/100)%10, (sol/10)%10, sol%10);
        if(ErrorMsg) continue;
        ptrChildren[idx]->last_move = sol;
        ptrChildren[idx]->Next_Turn();
        if(i<SolutionList.idx) idx++;
    }
    childNum = idx;
    if(childNum) isLeaf=false;
}

void ChessBoard::update(int val) {
    visits++;
    score += val;
}

int ChessBoard::selectChild() {
    if(win) return -1;
    int ret = 0;
    double bestScore = -786554453;
    for(int i=0; i<childNum; i++) { // 遍历所有子结点
        ChessBoard* ptrCurChild = ptrChildren[i]; 
        // 局面评估
        double curScore = ptrCurChild->score / (ptrCurChild->visits + EPS) + C * sqrt(log(visits + 1) / (ptrCurChild->visits + EPS)); // 加EPS防止除数为0的情况 
        if(curScore > bestScore) {
            ret = i;
            bestScore = curScore;
        }
    }
    return ret;
}

void ChessBoard::iterate() { // 遍历
    stack<ChessBoard*> visited;
    ChessBoard* ptrCur = this;
    visited.push(this);
    int bestChild = 0; // 选取最优的子节点
    while (!ptrCur->isLeaf && !ptrCur->win) {
        bestChild = ptrCur->selectChild();
        ptrCur = ptrCur->ptrChildren[bestChild];
        visited.push(ptrCur);
    } // 跳出这里时，ptrCur已经是叶子节点
    ptrCur->expand();
    bestChild = ptrCur->selectChild();
    ptrCur = ptrCur->ptrChildren[bestChild];
    visited.push(ptrCur);
    ptrCur->Judge_Win();
    int addscore = ptrCur->judgeScore() + win * 78653; // 给获胜分支加权放大
    while (!visited.empty()) {
        ptrCur = visited.top();
        ptrCur->update(addscore);   // 依次更新节点数值
        visited.pop();
    }
}

int ChessBoard::getBestSol() {
    uct_turnplayer = turn_player;
    for(int i=0; i<80; i++) // 共迭代几次 通过调整这个控制时间
        iterate();
    //int bestSolId = selectChild();
    int bestSolId = 0;
    for(int i=1; i<childNum; i++) {
        if(ptrChildren[childNum]->score && ptrChildren[childNum]->visits > ptrChildren[bestSolId]->visits) // 访问次数最多的即为最优解
            bestSolId = childNum;
    }
    if(bestSolId != -1) return ptrChildren[bestSolId]->last_move;
    else return -1;
}

void ChessBoard::kingMove(int color, int kingdist[8][8]) {
    queue<int> que; // int里装的是xy，意义同上
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++){
            kingdist[i][j] = 786554453; // INF
        }
    for(int chessidx=0; chessidx<4; chessidx++) {
        que.push(chess[color][chessidx]);
        kingdist[chess[color][chessidx]/10][chess[color][chessidx]%10] = 0;
        while(!que.empty()) {
            int xy = que.front();
            que.pop();
            for(int dir=0; dir<8; dir++) {
                int x_tmp = xy/10 + dx[dir], y_tmp = xy%10 + dy[dir];
                if(In_Board(x_tmp, y_tmp) && board[x_tmp][y_tmp] == EMPTY && kingdist[x_tmp][y_tmp] > kingdist[xy/10][xy%10] + 1) { // 类似最短路算法
                    que.push(x_tmp*10 + y_tmp);
                    kingdist[x_tmp][y_tmp] = kingdist[xy/10][xy%10] + 1;
                }
            }
        }
    }
}

void ChessBoard::queenMove(int color, int queendist[8][8]) {
    queue<int> que;
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++){
            queendist[i][j] = 786554453; // INF
        }
    for(int chessidx=0; chessidx<4; chessidx++) {
        que.push(chess[color][chessidx]);
        queendist[chess[color][chessidx]/10][chess[color][chessidx]%10] = 0;
        while(!que.empty()) {
            int xy = que.front();
            que.pop();
            for(int dir=0; dir<8; dir++) {
                int x_tmp = xy/10 + dx[dir], y_tmp = xy%10 + dy[dir];
                while(In_Board(x_tmp, y_tmp) && board[x_tmp][y_tmp] == EMPTY && queendist[x_tmp][y_tmp] > queendist[xy/10][xy%10] + 1) { // 类似最短路算法
                    que.push(x_tmp*10 + y_tmp);
                    queendist[x_tmp][y_tmp] = queendist[xy/10][xy%10] + 1;
                    x_tmp += dx[dir], y_tmp+= dy[dir];
                }
            }
        }
    }
}

int POW(int base,int num) // 快速幂
{
	if(!num) return 1;
    if(num >= 31) return 2147483647; // 防爆炸
	int x = POW(base, num/2);
	int ans= x * x;
	if(num % 2) ans = ans * base;
	return ans;
}

int ChessBoard::judgeScore() {
    // 计算territory值   默认是黑色→白色
    int k_b[8][8], q_b[8][8], k_w[8][8], q_w[8][8]; // 黑方与白方的kingmove与queenmove值数组
    kingMove(BLACK, k_b); kingMove(WHITE, k_w);
    queenMove(BLACK, q_b); queenMove(WHITE, q_w);
    float t1 = 0, t2 = 0, p1 = 0, p2 = 0; //计算position(c1,c2)
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            // 计算queenMove带来的territory改变
            if(q_b[i][j] == 786554453 && q_b[i][j] == q_w[i][j]) t1+=0; // 都无法走到
            else if (q_b[i][j] != 786554453 && q_b[i][j] == q_w[i][j]) t1 += firstbonus; // 都能在相同步数内走到，这时需区分先手
            else if(q_w[i][j] < q_b[i][j]) t1 += -1; // 白棋步数用得更少
            else  t1 += 1; // 黑棋用的步数更少
			// 计算kingMove带来的territory改变
            if(k_b[i][j] == 786554453 && k_b[i][j] == k_w[i][j]) t2+=0; // 都无法走到 
            else if (k_b[i][j] != 786554453 && k_b[i][j] == k_w[i][j]) t2 += firstbonus; // 同上
            else if(k_w[i][j]<k_b[i][j]) t2 += -1;
            else t2 += 1;

			p1 += 2 * (1.0 / POW(2, q_b[i][j]) - 1.0 / POW(2, q_w[i][j]));
			p2 += min(1.0, max(-1.0, (1.0 / 6.0) * (k_w[i][j] - k_b[i][j])));
        }
    }
    // 计算mobility
    int emptyNearby[8][8];
	for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            emptyNearby[i][j] = 0;
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            if(board[i][j] == EMPTY) { // 是空的
                for(int dir=0; dir<8; dir++){ //八个方向
                    if(In_Board(i+dx[dir],j+dy[dir]) && board[i+dx[dir]][j+dy[dir]]==0) emptyNearby[i][j]++;
                }
            }
        }
    }
    float m_b = 0, m_w = 0; // 黑棋与白棋的mobility
    // 计算黑棋
    for(int idx=0; idx<4; idx++) { // 第idx个棋
        for (int dir=0; dir<8; dir++) { // 向8个方向
            for(int step = 1;step < 8;step++) { // 步长
                int nx = chess[BLACK][idx] / 10 + dx[dir]*step, ny = chess[BLACK][idx] % 10 + dy[dir]*step;
                if (In_Board(nx, ny) && board[nx][ny] == EMPTY && q_b[nx][ny] != 786554453) { //判断是否可以移动
                    m_b += (float)emptyNearby[nx][ny] / (float)step;
                } else break; //此方向不能继续移动
            }
        }
    }
    // 计算白棋
    for(int idx=0; idx<4; idx++) { // 第idx个棋
        for (int dir=0; dir<8; dir++) { // 向8个方向
            for(int step = 1;step < 8;step++) { // 步长
                int nx = chess[WHITE][idx] / 10 + dx[dir]*step, ny = chess[WHITE][idx] % 10 + dy[dir]*step;
                if (In_Board(nx, ny) && board[nx][ny] == EMPTY && q_w[nx][ny] != 786554453) { //判断是否可以移动
                    m_w += (float)emptyNearby[nx][ny] / (float)step;
                } else break; //此方向不能继续移动
            }
        }
    }
    // 计算score
    float ret = 0;
    if(turns < 20) {
        ret = k1[0] * t1 + k2[0] * t2 + k3[0] * p1 + k4[0] * p2 + k5[0]*(m_b-m_w);
    } else if(turns < 50) {
        ret = k1[1] * t1 + k2[1] * t2 + k3[1] * p1 + k4[1] * p2 + k5[1]*(m_b-m_w);
    } else {
        ret = k1[2] * t1 + k2[2] * t2 + k3[2] * p1 + k4[2] * p2 + k5[2]*(m_b-m_w);
    }
    if(uct_turnplayer == BLACK) return ret;
    else return -ret;
}

int main() {
    ChessBoard Board;
    Board.Reset();
    int turn_num; cin >> turn_num;
    int x_start, y_start, x_final, y_final, x_block, y_block;
    cin >> x_start >> y_start >> x_final >> y_final >> x_block >> y_block;
    if(x_start == -1) Board.turn_player = 1;
    else {
        Board.turn_player = 1;
        Board.Move(y_start, x_start, y_final, x_final, y_block, x_block); // 适应接口，需要换序
        Board.turn_player = 2;
    }
    for(int i=1; i<=2*(turn_num-1); i++) {
        cin >> x_start >> y_start >> x_final >> y_final >> x_block >> y_block;
        Board.Move(y_start, x_start, y_final, x_final, y_block, x_block); // 适应接口，需要换序
        Board.Next_Turn();
    }
    int sol = Board.getBestSol();
    cout << (sol/10000)%10 << " " << sol/100000 << " " << (sol/100)%10 << " " << (sol/1000)%10 << " " << sol%10 << " " << (sol/10)%10 << endl;
    return 0;
}

