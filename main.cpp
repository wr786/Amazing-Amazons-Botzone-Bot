#include "amazons.h"

// 待实现功能备忘录
// 1. 菜单Menu √
// 2. 悔棋Regret √
// 3. 提示Hint
// 4. AI
// 5. GUI

int main() {
    ChessBoard Board; Board.Reset();
    char op; int sx, sy, ex, ey, bx, by, winner;
    cout << "请问您要读取保存的存档吗？Y/N\n";
    cin >> op; if(op == 'Y') Board.Reload();
    while(!(winner = Board.Judge_Win())) {
        Board.Display(); 
        Board.Show_Menu(); cin >> op;
        if(op == 'M') {
            cout << "请按 出发点x坐标 出发点y坐标 目的地x坐标 目的地y坐标 障碍物x坐标 障碍物y坐标 （第x列第y行）的格式输入操作：\n";
            cin >> sx >> sy >> ex >> ey >> bx >> by;
            int ErrorMessage = Board.Move(sx-1, sy-1, ex-1, ey-1, bx-1, by-1);
            if(!ErrorMessage)   Board.Next_Turn();
            else{
                cout << "输入Y继续，否则退出游戏:\n"; cin >> op;
                if(op != 'Y') return 0;
            } 
        } else if(op == 'S') {
            Board.Save();
        } else if(op == 'R') {
            Board.Regret(sx-1, sy-1, ex-1, ey-1, bx-1, by-1);
        }   
    }
    system("pause");
    return 0;
}
