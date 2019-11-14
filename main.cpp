#include "amazons.h"

int main() {
    ChessBoard Board;
    Board.Reset();
    int sx, sy, ex, ey, bx, by, winner;
    while(!(winner = Board.Judge_Win())) {
        Board.Display();
        cout << "\n现在是";
        Board.turn_player == 1? cout << "黑方@" : cout << "白方O";
        cout << "のTurnだ！\n";
        cout << "请按 出发点x坐标 出发点y坐标 目的地x坐标 目的地y坐标 障碍物x坐标 障碍物y坐标 （第x列第y行）的格式输入操作：\n";
        cin >> sx >> sy >> ex >> ey >> bx >> by;
        int ErrorMessage = Board.Move(sx-1, sy-1, ex-1, ey-1, bx-1, by-1);
        if(!ErrorMessage)   Board.Next_Turn();
        else{
            cout << "输入Y继续，否则退出游戏:\n";
            char op; cin >> op;
            if(op != 'Y') return 0;
        } 
    }
    return 0;
}
