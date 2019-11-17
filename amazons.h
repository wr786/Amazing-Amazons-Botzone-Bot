#include<cstdio>
#include<iostream>
#include<cmath>
#include<vector>
using namespace std;

class ChessBoard {
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
        struct SL { // 存放可行解用
            int solution[2000];
            int idx = 0; // 记得每次用完时要归零
        } SolutionList;
        void Display();
        void Reset();
        bool In_Board(int x, int y);
        bool Can_Move(int x, int y);
        int Move(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block);
        bool Judge_Win();
        void Next_Turn();
        void Save(); // 存档
        void Reload(); // 读档
        void Show_Menu(); // 菜单展示
        void Regret(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block); // 悔棋
        void Reset_Hint();
        void Search_Hint(int x, int y, int dir);
        void Hint(); // 提示（同时也是计算出所有可以前往的点） 面向用户使用
        // 用来寻找所有可以落子和放障碍物的点 面向AI寻找可行解使用
        // 其中SolutionList存放的每个Solution均为一个整数，每位数依次为 起始点x、y，终点x、y，障碍点x、y
        inline void Find_Possible_Move(int xy);
        inline void Find_Possible_Block(int xy_start, int xy_final);
        inline void Find_Solutions();
};

void ChessBoard::Display() { // demonstrate
    system("cls"); // 清屏
    // ● ○ ▓ ◎
    for(int i=0; i<=8; i++) cout << i << "\t"; cout << "\n\n"; // 输出横坐标
    for(int i=0; i<8; i++) { // 中间行
        cout << i + 1; // 输出纵坐标
        for(int j=0; j<8; j++) {
            cout << "\t";
            if(board[i][j] == BLACK) cout << "●"; // 黑棋
            if(board[i][j] == WHITE) cout << "○"; // 白棋
            if(board[i][j] == EMPTY) cout << " ";
            if(board[i][j] == BLOCK) cout << "▓"; // 障碍
            if(board[i][j] == CANGO) cout << "◎"; // 提示点
        }
        cout << "\n\n";
    }
    if(hinted) Reset_Hint();
}

void ChessBoard::Reset() { // initialize
    ios::sync_with_stdio(false); // iostream加速
    system("chcp 65001"); // 使cmd能输出UTF-8编码
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            board[i][j] = 0; 
    board[0][2] = board[2][0] = board[0][5] = board[2][7] = BLACK;
    board[5][0] = board[7][2] = board[5][7] = board[7][5] = WHITE;
    chess[1][0] = 2; chess[1][1] = 20; chess[1][2] = 5; chess[1][3] = 27;
    chess[2][0] = 50; chess[2][1] = 72; chess[2][2] = 57; chess[2][3] = 75;
    turn_player = 1;
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

int ChessBoard::Move(int y_start, int x_start, int y_final, int x_final, int y_block, int x_block) { // 按接口要求，需要转置
    // 错误判断
    if(board[x_start][y_start] != turn_player) {
        cout << "非法坐标：这个位置没有您的棋！ErrorType:11037\n";
        return 11037;
    }
    if(!In_Board(x_start, y_start) && !In_Board(x_final, y_final) && !In_Board(x_block, y_block)) {
        cout << "非法坐标：坐标越界！ErrorType:37510\n";
        return 37510;
    }
    if((board[x_final][y_final] || board[x_block][y_block]) && !(x_block == x_start && y_block == y_start)) { // 特判回马枪情形
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
    board[x_start][y_start] = 0; 
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
        Display();
        cout << "胜者是白方○！\n";
    } else if(locked[WHITE][0]) {
        Display();
        cout << "胜者是黑方●！\n";
    } else return false;
    return true;
}

void ChessBoard::Next_Turn() {
    turn_player = 3 - turn_player;
}

void ChessBoard::Save() { // 直接保存棋盘信息
    FILE* fp = fopen("./Data/archive.log", "w");
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            fprintf(fp, "%d ", board[i][j]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "%d\n", turn_player);
    fclose(fp);
}

void ChessBoard::Reload() { // 直接读取棋盘信息
    FILE* fp = fopen("./Data/archive.log", "r");
    if(!feof(fp)) {
        for(int i=0; i<8; i++)
            for(int j=0; j<8; j++)
                fscanf(fp, "%d", &board[i][j]);
    }
    fscanf(fp, "%d", &turn_player);
    fclose(fp);
}

void ChessBoard::Show_Menu() {
    cout << "\n\t\t\t现在是" << (turn_player == 1? "黑方●" : "白方○") << "のTurnだ☆ぜ！\n";
    cout << "\t\t\t> M  移动\t> R  悔棋\t\n";
    cout << "\t\t\t> S  存档\t> H  提示\t\n";
    cout << "\t\t\t> E  退出\t> N  是新游戏\t\n";
}

void ChessBoard::Regret(int y_start, int x_start, int y_final, int x_final, int y_block, int x_block) { // 按接口要求，需要转置
    // 因为坐标的合法性已经在Move里验证过了，所以此处可以免去错误检测环节
    // ワタシ、再生産。
    Next_Turn();
    board[x_start][y_start] = turn_player; 
    board[x_final][y_final] = 0;
    board[x_block][y_block] = 0;
}

void ChessBoard::Reset_Hint() {
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            if(board[i][j] == CANGO)
                board[i][j] = 0; // EMPTY;
    hinted = false;
}

void ChessBoard::Search_Hint(int x, int y, int dir_limited) {
    if(dir_limited == -1) { // 没有规定方向
        for(int dir=0; dir<8; dir++) {
            int x_next = x + dx[dir], y_next = y + dy[dir];
            if(In_Board(x_next, y_next) && !board[x_next][y_next] && Can_Move(x_next, y_next)) { // 在棋盘内 + 为空 + 可以放障碍物
                board[x_next][y_next] = CANGO;
                hinted = true;
                Search_Hint(x_next, y_next, dir);
            }
        }
    } else {
        int x_next = x + dx[dir_limited], y_next = y + dy[dir_limited];
        if(In_Board(x_next, y_next) && !board[x_next][y_next] && Can_Move(x_next, y_next)) { // 在棋盘内 + 为空 + 可以放障碍物
            board[x_next][y_next] = CANGO;
            Search_Hint(x_next, y_next, dir_limited);
        }
    }
}

void ChessBoard::Hint() {
    for(int i=0; i<4; i++)
        Search_Hint(chess[turn_player][i]/10, chess[turn_player][i]%10, -1);
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
