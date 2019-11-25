#include<cstdio>
#include<iostream>
#include<algorithm>
#include<cmath>
#include<vector>
#include<stack>
#include<queue>
#include<cstdlib>
#include<set>
#include<vector>
#include<ctime>
using namespace std;
//#define EPS 1e-4
#define C 0.35
#define PERMUTATION_4_MAX 24
#define PERMUTATION_8_MAX 40320
// 蓄水池抽样算法预处理
int PERM8[PERMUTATION_8_MAX][8] = {

};
int PERM4[PERMUTATION_4_MAX][4] = {
{0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},{1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},{2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},{3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0}
};  

class ChessBoard { // 每个棋盘都是UCTree的一个节点！（暴论）
    private:
        int board[8][8]; // 棋盘本盘
        //           → ↘  ↓  ↙ ←  ↖   ↑   ↗
        int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
        int dy[8] = {1, 1, 0, -1, -1, -1, 0, 1};
        int chess[3][4]; // 存取四个棋所在的xy坐标，1为黑棋，2为白棋,十位数为x，个位数为y
        bool hinted = false;
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
        void Initialize();
        void Reset();
        bool In_Board(int x, int y);
        bool Can_Move(int x, int y);
        int Move(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block);
        void Regret(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block); // 悔棋
        bool Judge_Win();
        void Next_Turn();
        // 用来寻找所有可以落子和放障碍物的点 面向AI寻找可行解使用
        // 其中SolutionList存放的每个Solution均为一个整数，每位数依次为 起始点x、y，终点x、y，障碍点x、y
        inline void Find_Possible_Move(int xy);
        inline void Find_Possible_Block(int xy_start, int xy_final);
        inline void Find_Solutions();
        // UCT(MCTS) section!
        ChessBoard* child[2001];  //子结点
        int childNum = 0;
        int last_move;  // 同solution表示方法
        float visits = 1; // 访问次数
        double score = 0;  //估值
        int turns = 0; // 第几回合
        int uct_turnplayer; 
        set<int> AlreadyMoved;
        inline void copy(const ChessBoard &temp);
        inline int getRndSol(ChessBoard *Node); // 用蓄水池算法获取随机数
        ChessBoard* select();
        ChessBoard* expand();
        void UCTSearch();
        inline bool fullyExpand();
        inline bool isEnd();
        inline void kingMove(int color, int kingdist[8][8]);
        inline void queenMove(int color, int queendist[8][8]);
        inline double evaluate();
        // 参数列表
        float k1[3] = {0.37, 0.25, 0.10};
        float k2[3] = {0.14, 0.30, 0.80};
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
    if(!In_Board(x_start, y_start) || !In_Board(x_final, y_final) || !In_Board(x_block, y_block)) {
        cout << "非法坐标：坐标越界！ErrorType:37510\n";
        return 37510;
    }
    if(board[x_start][y_start] != turn_player) {
        cout << "非法坐标：这个位置没有您的棋！ErrorType:11037\n";
        //cout << "board[" << x_start << "][" << y_start << "] = " << board[x_start][y_start];
        return 11037;
    }
    if(board[x_final][y_final] || (board[x_block][y_block] && !(x_block == x_start && y_block == y_start))) { // 特判回马枪情形
        cout << "非法坐标：需求坐标已被占用！ErrorType:23333\n";
        return 23333;
    }
    int deltax = abs(x_final - x_start), deltay = abs(y_final - y_start);
    if(!(deltax == 0) && !(deltay == 0) && !(deltax == deltay)) { // 移动合法性判断
        cout << "非法移动：棋子移动方法不在8个方向上！ErrorType:88888\n";
        return 88888;
    }
    deltax = abs(x_block - x_final), deltay = abs(y_block - y_final);
    if(!(deltax == 0) && !(deltay == 0) && !(deltax == deltay)) { // 障碍物合法性判断
        cout << "非法障碍：障碍物摆放方法不在8个方向上！ErrorType:10086\n";
        return 88889;
    }
    // 坐 标 移 动
    int chessidx = 0;
    for(;chessidx<4;chessidx++) {if(chess[turn_player][chessidx] == x_start*10 + y_start) break;}
    board[x_start][y_start] = EMPTY; 
    board[x_final][y_final] = turn_player;
    board[x_block][y_block] = BLOCK;
    chess[turn_player][chessidx] = x_final*10 + y_final;
    return 0;
}

void ChessBoard::Regret(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block) { // 按接口要求，需要转置
    // 因为坐标的合法性已经在Move里验证过了，所以此处可以免去错误检测环节
    // ワタシ、再生産。
    int chessidx = 0; // 这个也该对应地改变
    for(;chessidx<4;chessidx++) {if(chess[turn_player][chessidx] == x_final*10 + y_final) break;}
    chess[turn_player][chessidx] = x_start*10 + y_start;
    board[x_final][y_final] = EMPTY;
    board[x_block][y_block] = EMPTY;
    board[x_start][y_start] = turn_player;  // 顺序不能变！
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
        else win = -1;
        //Display();
        //cout << "胜者是白方○！\n";
    } else if(locked[WHITE][0]) {
        if(uct_turnplayer == BLACK) win = 1;
        else win = -1;
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
inline void ChessBoard::copy(const ChessBoard &temp){ // 拷贝函数，只考虑必要信息
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

inline int ChessBoard::getRndSol(ChessBoard *Node) { // 随机Roll一个solution出来，增加随机性
    // 用数组指针方便维护
    int *list1 = PERM4[rand() % PERMUTATION_4_MAX], // 第随机个棋子
        *list2 = PERM8[rand() % PERMUTATION_8_MAX], // 第一步的随机方向
        *list3 = PERM8[rand() % PERMUTATION_8_MAX]; // 第二步的随机方向
    int x_start, y_start, x_final, y_final, x_block, y_block;
    for (int i = 0; i < 4; i++) {
        int rnd_chessidx = list1[i]; // 随机选取当前方的第list1[i]个棋子
        x_start = chess[turn_player][rnd_chessidx] / 10, y_start = chess[turn_player][rnd_chessidx] % 10;
        for (int dir1 = 0; dir1 < 8; dir1++) { 
            // rnd_dir1才是是随机出来的方向
            int rnd_dir1 = list2[dir1], CNT = 0;  // CNT为第一步步法计数
            int retsol; // 返回值
            x_final = x_start + dx[rnd_dir1], y_final = y_start + dy[rnd_dir1];
            while(In_Board(x_final, y_final) && board[x_final][y_final] == EMPTY) {
                for (int dir2 = 0; dir2 < 8; dir2++) {
                int rnd_dir2 = list3[dir2], cnt = 0;  //cnt为第二步步法计数
                int sol;
                x_block = x_final + dx[rnd_dir2], y_block = y_final + dy[rnd_dir2];
                while(In_Board(x_block, y_block) && (board[x_block][y_block] == EMPTY || (x_block == x_start && y_block == y_start))) {
                    // 找到了一个sol
                    int tmpsol = x_start*1e5 + y_start*1e4 + x_final*1e3 + y_final*1e2 + x_block*10 + y_block;
                    if (Node == nullptr || !Node->AlreadyMoved.count(tmpsol))
                    // 以一定的概率接受（蓄水池）
                    if (!(rand() % (++cnt))) sol = tmpsol;
                    x_block += dx[rnd_dir2], y_block += dy[rnd_dir2];
                }
                if (cnt)
                    if (!(rand() % (++CNT))) retsol = sol;
                }
                x_final += dx[rnd_dir1], y_final += dy[rnd_dir1];
            }
            if (CNT) return retsol;
        }
    }
    // 没有可走的方法
    return -111111; // 等价于 -1 -1 -1 -1 -1 -1
}

inline void ChessBoard::kingMove(int color, int kingdist[8][8]) { // 计算kingMove值
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

inline void ChessBoard::queenMove(int color, int queendist[8][8]) { // 计算queenMove值
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

int POW(int base,int num) { // 快速幂 
	if(!num) return 1;
    if(base == 2 && num >= 31) return 2147483647; // 防爆炸
	int x = POW(base, num/2);
	int ans= x * x;
	if(num % 2) ans = ans * base;
	return ans;
}

inline double ChessBoard::evaluate() {
    stack<int> remem; // 记忆步法，以便还原棋局
    int tmpcolor = turn_player, SIM; // 暂时存储当前颜色与模拟次数
    if (turn_player == uct_turnplayer) SIM = 4;
    else SIM = 3;
    // 模拟SIM次后再进行评估
    for (int sim = 0; sim < SIM; sim++) {
        int sol = getRndSol(nullptr);
        if(sol < 0) break; // 没有可行解了
        Move(sol/100000, (sol/10000)%10, (sol/1000)%10, (sol/100)%10, (sol/10)%10, sol%10);
        remem.push(sol);
        tmpcolor = 3 - tmpcolor; // 模拟换回合
    }
    double ret = 0;  // return value
    // 计算territory值 （这里默认从黑方的视角来计算）
    // 计算kingMove与queenMove值
    int kingdist_black[8][8], kingdist_white[8][8];
    int queendist_black[8][8], queendist_white[8][8];
    kingMove(BLACK, kingdist_black); kingMove(WHITE, kingdist_white);
    queenMove(BLACK, queendist_black); queenMove(WHITE, queendist_white);
    double t1 = 0, t2 = 0; // 分别代表两种territory值
    double p1 = 0, p2 = 0; // 分别代表两种position值
    for (int i=0; i<8; i++)
        for (int j=0; j <8; j++) {
            // 如果都能走到，且需求步数相同，先手优势。
            if (kingdist_black[i][j] == kingdist_white[i][j] && kingdist_white[i][j] != 786554453) 
                t1 += (tmpcolor == BLACK ? 0.2 : -0.2);
            // 处于黑色控制
            else if (kingdist_black[i][j] < kingdist_white[i][j])
                t1 += 1;
            // 处于白色控制
            else if (kingdist_black[i][j] > kingdist_white[i][j])
                t1 -= 1;
            // queenMove同理
            if (queendist_black[i][j] == queendist_white[i][j] && queendist_white[i][j] != 786554453)
                t2 += (tmpcolor == BLACK ? 0.2 : -0.2);
            else if (queendist_black[i][j] < queendist_white[i][j])
                t2 += 1;
            else if (queendist_black[i][j] > queendist_white[i][j])
                t2 -= 1;
            // Position参数
            p1 += 2 * (1.0 / POW(2, queendist_black[i][j]) - 1.0 / POW(2, queendist_white[i][j]));
            p2 += min(1.0, max(-1.0, (1.0 / 6.0) * (kingdist_white[i][j] - kingdist_black[i][j])));
        }
    // 计算mobility
    int emptyNearby[8][8]; // 先计算每个空格位附能达到的空格数
	for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            emptyNearby[i][j] = 0;
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            if(board[i][j] == EMPTY) { // 是空的
                for(int dir=0; dir<8; dir++){ //八个方向
                    if(In_Board(i+dx[dir],j+dy[dir]) && board[i+dx[dir]][j+dy[dir]] == EMPTY) emptyNearby[i][j]++;
                }
            }
        }
    }
    double m_b = 0, m_w = 0; // 黑棋与白棋的mobility
    double minBlack = 786554453, minWhite = 786554453; // 最小灵活度，防止过早堵死
    // 计算黑棋
    for(int idx=0; idx<4; idx++) { // 第idx个棋
        double m_b_tmp = 0;
        for (int dir=0; dir<8; dir++) { // 向8个方向
            for(int step = 1; step < 8; step++) { // 步长
                int nx = chess[BLACK][idx] / 10 + dx[dir]*step, ny = chess[BLACK][idx] % 10 + dy[dir]*step;
                if (In_Board(nx, ny) && board[nx][ny] == EMPTY && queendist_black[nx][ny] != 786554453) { //判断是否可以移动
                    m_b_tmp += (float)emptyNearby[nx][ny] / (float)step;
                } else break; //此方向不能继续移动
            }
        }
        m_b += m_b_tmp;
        minBlack = min(minBlack, m_b_tmp);
    }
    // 计算白棋
    for(int idx=0; idx<4; idx++) { // 第idx个棋
        double m_w_tmp = 0;
        for (int dir=0; dir<8; dir++) { // 向8个方向
            for(int step = 1; step < 8; step++) { // 步长
                int nx = chess[WHITE][idx] / 10 + dx[dir]*step, ny = chess[WHITE][idx] % 10 + dy[dir]*step;
                if (In_Board(nx, ny) && board[nx][ny] == EMPTY && queendist_white[nx][ny] != 786554453) { //判断是否可以移动
                    m_w_tmp += (float)emptyNearby[nx][ny] / (float)step;
                } else break; //此方向不能继续移动
            }
        }
        m_w += m_w_tmp;
        minWhite = min(minWhite, m_w_tmp);
    }
    double m = m_b + minBlack - m_w - minWhite;
    // 复原棋盘
    while(!remem.empty()) {
        int sol = remem.top();
        Regret(sol/100000, (sol/10000)%10, (sol/1000)%10, (sol/100)%10, (sol/10)%10, sol%10);
        remem.pop();
    }
    //分段评估
    if (turns <= 20)
        ret = k1[0] * t1 + k2[0] * t2 + k3[0] * p1 + k4[0] * p2 + k5[0] * m;
    else if (turns < 50)
        ret = k1[1] * t1 + k2[1] * t2 + k3[1] * p1 + k4[1] * p2 + k5[1] * m;
    else
        ret = k1[2] * t1 + k2[2] * t2 + k3[2] * p1 + k4[2] * p2 + k5[2] * m;
    if (uct_turnplayer == BLACK)
        return ret;
    else
        return -ret;
}

ChessBoard* ChessBoard::select() {
    ChessBoard* ret = nullptr;
    double bestScore = -786554453;
    // Minimax选择：如果这一步是对方下就选最小的，否则选最大的。
    for (ChessBoard* c : child) { // 遍历每个子节点
        // 这里的0.35是常数C，可以修改来改变搜索的深度与广度
        double curScore = (turn_player == uct_turnplayer ? c->score / c->visits : 0.35 * sqrt(log(visits) / c->visits));
        if (curScore > bestScore) {
            ret = c;
            bestScore = curScore;
        }
    }
    return ret;
}

ChessBoard* ChessBoard::expand() {
    int sol = getRndSol(this); // 对现在的局面生成随机可行解
    if(sol < 0) return nullptr; // 没有可行解了
    // expand 子节点
    ChessBoard* node = new ChessBoard;
    node->copy(*this);
    node->Move(sol/100000, (sol/10000)%10, (sol/1000)%10, (sol/100)%10, (sol/10)%10, sol%10);
    node->last_move = sol;
    node->Next_Turn();
    node->score = evaluate();
    child[childNum++] = node;
    AlreadyMoved.insert(sol); // 避免再次随机到同一个
    return node;
}

// 蒙特卡洛搜索主函数
void ChessBoard::UCTSearch() {
    vector<ChessBoard *> visited; // 不用queue是因为要遍历两次
    ChessBoard* cur = this;
    visited.push_back(cur);
    // 结点已经被完全扩展而且不是最后结点，就向下继续模拟
    while (cur->fullyExpand() && !cur->isEnd()) {
        cur = cur->select();
        // 模拟移动
        int sol = cur->last_move;
        // 注意，这里move的不是子节点！
        // 这里需不需要Next_Turn待定！
        Move(sol/100000, (sol/10000)%10, (sol/1000)%10, (sol/100)%10, (sol/10)%10, sol%10);
        //记录走过的位点
        visited.push_back(cur);
    }
    // 扩展深度
    ChessBoard* newNode = cur->expand();
    if (newNode != nullptr) { // 这个结点可以扩展（即不为叶子节点）
        //访问新结点
        visited.push_back(newNode);
        //反向传播，由于估值时已经初始化，root和新结点均不估值
        for (int i = (int)visited.size() - 2; i >= 0; i--) {
        visited[i]->visits++; // 被访问，所以增加访问次数
        visited[i]->score += visited[(int)visited.size() - 1]->score; // 加上叶子节点的score
        }
    }
    //恢复棋盘
    for (int i = (int)visited.size() - 1; i >= 1; i--) {
        int sol = visited[i]->last_move;
        // 这里需不需要Next_Turn待定！
        Regret(sol/100000, (sol/10000)%10, (sol/1000)%10, (sol/100)%10, (sol/10)%10, sol%10);
    }
    }

inline bool ChessBoard::fullyExpand() { // 此结点是否已经完全扩展
    if (SolutionList.idx == 0) Find_Solutions();
    return SolutionList.idx <= childNum; // 子节点个数＞所有可能方法数，则已完全扩展
}

inline bool ChessBoard::isEnd() { // 此结点是否还能扩展，即是否为叶子节点
    if (SolutionList.idx == 0) Find_Solutions();
    return SolutionList.idx == 0; // 没有可行解，那么就不能继续了
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
        Board.Next_Turn();
    }
    for(int i=1; i<=2*(turn_num-1); i++) {
        cin >> x_start >> y_start >> x_final >> y_final >> x_block >> y_block;
        Board.Move(y_start, x_start, y_final, x_final, y_block, x_block); // 适应接口，需要换序
        Board.Next_Turn();
    }
    Board.turns = turn_num;
    srand(time(NULL)); // 重置随机数种子
    auto start = (double)clock(); // 进行计时，防止超时并能进行最深的迭代次数
    // 第一轮2秒，否则1秒
    while ((double)clock() - start < (turn_num == 1 ? 1.92 : 0.92) * CLOCKS_PER_SEC)
        Board.UCTSearch();
    // 输出结果
    ChessBoard* result = Board.select();
    if (result != nullptr) {
        int sol = result->last_move;
        cout << (sol/10000)%10 << " " << sol/100000 << " " << (sol/100)%10 << " " << (sol/1000)%10 << " " << sol%10 << " " << (sol/10)%10 << endl;
    } else cout << "-1 -1 -1 -1 -1 -1\n";//没有走法说明自己输了，特判以防程序崩溃
    return 0;
}