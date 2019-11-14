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
        cout << "请按 出发点x坐标 出发点y坐标 目的地x坐标 目的地y坐标 障碍物x坐标 障碍物y坐标 （从1开始）的格式输入操作：\n";
        cin >> sx >> sy >> ex >> ey >> bx >> by;
        int ErrorMessage = Board.Move(sx-1, sy-1, ex-1, ey-1, bx-1, by-1);
        if(!ErrorMessage)   Board.Next_Turn();
    }
    Board.Display();
    if(winner = Board.BLACK) cout << "胜者是黑方@!\n";
    else cout << "胜者是白方O!\n";
    return 0;
}
