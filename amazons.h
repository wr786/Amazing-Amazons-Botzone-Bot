#include<cstdio>
#include<iostream>
using namespace std;

class ChessBoard {
    private:
        int board[8][8]; // 棋盘本盘
        //           → ↘  ↓  ↙ ←  ↖   ↑   ↗
        int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
        int dy[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    public:
        // 标记
        short BLACK = 1; // 黑棋
        short WHITE = 2; // 白棋
        short BLOCK = -1; // 障碍物
        int turn_player; // 本回合玩家
        void Display();
        void Reset();
        int Move(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block);
        int Judge_Win();
        void Next_Turn();
};

void ChessBoard::Display() { // demonstrate
    system("cls"); // 清屏
    // ● ○ ▓
    for(int i=0; i<=8; i++) cout << i << " "; cout << endl; // 输出横坐标
    for(int i=0; i<8; i++) { // 中间行
        cout << i + 1; // 输出纵坐标
        for(int j=0; j<8; j++) {
            cout << " ";
            if(board[i][j] == 1) cout << "●"; // 黑棋
            if(board[i][j] == 2) cout << "○"; // 白棋
            if(board[i][j] == 0) cout << " ";
            if(board[i][j] == -1) cout << "▓"; // 障碍
        }
        cout << "\n";
    }
}

void ChessBoard::Reset() { // initialize
    ios::sync_with_stdio(false); // iostream加速
    system("chcp 65001"); // 使cmd能输出UTF-8编码
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            board[i][j] = 0; 
    board[0][2] = board[2][0] = board[0][5] = board[2][7] = BLACK;
    board[5][0] = board[7][2] = board[5][7] = board[7][5] = WHITE;
    turn_player = 1;
}

bool in_board(int x, int y) {
    if(x < 0 || x >= 8) return false;
    if(y < 0 || y >= 8) return false;
    return true;
}

int ChessBoard::Move(int x_start, int y_start, int x_final, int y_final, int x_block, int y_block) {
    // 错误判断
    if(board[x_start][y_start] != turn_player) {
        cout << "非法坐标：这个位置没有您的棋！ErrorType:11037\n";
        return 11037;
    }
    if(!in_board(x_start, y_start) && !in_board(x_final, y_final) && !in_board(x_block, y_block)) {
        cout << "非法坐标：坐标越界！ErrorType:37510\n";
        return 37510;
    }
    if(board[x_final][y_final] || board[x_block][y_block]) {
        cout << "非法坐标：需求坐标已被占用！ErrorType:23333\n";
        return 23333;
    }
    // 坐 标 移 动
    board[x_start][y_start] = 0; 
    board[x_final][y_final] = turn_player;
    board[x_block][y_block] = BLOCK;
    return 0;
}

int ChessBoard::Judge_Win() {
    int cnt_black = 0, cnt_white = 0, color_now;
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++) {
            if(board[i][j] == BLACK) 
                cnt_black++, color_now = BLACK;
            else if(board[i][j] == WHITE)
                cnt_white++, color_now = WHITE;
            else continue;
            bool found_winner = true;
            for(int dir=0; dir<8; dir++) {
                int x_next = i + dx[dir];
                int y_next = j + dy[dir];
                if(!in_board(x_next, y_next)) { // 越界判断
                    found_winner = false;
                    break;
                }
                if(!board[x_next][y_next]) { // 是否被包围
                    found_winner = false;
                    break;
                }
            }
            if(found_winner) return color_now;
        }
    return 0;
}

void ChessBoard::Next_Turn() {
    turn_player = 3 - turn_player;
}