#include<cstdio>
#include<iostream>
#include<cmath>
#include<vector>
#include<stack>
using namespace std;
#define C 1 // 系数，通过调整这个改变搜索的深度与广度
#define EPS 1e-7

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
        // UCT部分
        ChessBoard* ptrChildren[2000];
        bool isLeaf = true;
        double visits = 0;
        double wins = 0; // 获胜次数
        // 关于局面评估
        // score = wins_child / visits_child + C*sqrt(log(visits + 1) / visits_child)
        // 其中wins_child / visits_child代表胜率
        int childNum = 2000;
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
}

void ChessBoard::expand() {
    if(!isLeaf) return;
    Find_Solutions();
    childNum = SolutionList.idx;
    int idx = 0;
    for (int i = 0; i < childNum; i++) { // 生成所有下了可行解的棋盘
        ptrChildren[i] = new ChessBoard();
        ptrChildren[i]->copy(*this);
        int sol = SolutionList.solution[i+1]; // 统一下标格式
        // 模拟下棋
        //cout << sol << endl;
        ptrChildren[i]->Move(sol/100000, (sol/10000)%10, (sol/1000)%10, (sol/100)%10, (sol/10)%10, sol%10);
        ptrChildren[i]->Next_Turn();
    }
    if(childNum) isLeaf=false;
}

void ChessBoard::update(int val) {
    visits++;
    wins += val;
}

int ChessBoard::selectChild() {
    int ret = 0;
    double bestScore = -786554453;
    for(int i=0; i<childNum; i++) { // 遍历所有子结点
        ChessBoard* ptrCurChild = ptrChildren[i]; 
        // 局面评估
        double curScore = (ptrCurChild->wins + EPS) / (ptrCurChild->visits + EPS) + C * sqrt(log(visits + 1) / (ptrCurChild->wins + EPS)); // 加EPS防止除数为0的情况 
        if(curScore >= bestScore) {
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
    while (!ptrCur->isLeaf) {
        bestChild = ptrCur->selectChild();
        ptrCur = ptrCur->ptrChildren[bestChild];
        visited.push(ptrCur);
    } // 跳出这里时，ptrCur已经是叶子节点
    ptrCur->expand();
    bestChild = ptrCur->selectChild();
    ptrCur = ptrCur->ptrChildren[bestChild];
    visited.push(ptrCur);
    Judge_Win(); // 这里需要判断是否获胜
    int val = win; // 记录胜负，自下而上回溯
    while (!visited.empty()) {
        ptrCur = visited.top();
        ptrCur->update(val);   // 依次更新节点数值
        visited.pop();
    }
}

int ChessBoard::getBestSol() {
    uct_turnplayer = turn_player;
    for(int i=0; i<40; i++) // 共迭代几次
        iterate();
    int bestSolId = selectChild();
    return SolutionList.solution[bestSolId];
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