#include "amazons.h"
#include<ctime> // 仅仅测试效率用！！！！！

// 待实现功能备忘录
// 1. AI
// 2. GUI
// 3. 分离判断可落子点与可放障碍物点（重复调用两次同一个函数即可）
// 4. 改进生成可行解函数的效率（可以尝试打个表？）（算了，打表空间复杂度太高）

int main() {
    ChessBoard Board; Board.Reset();
    char op; int sx, sy, ex, ey, bx, by, winner;
    cout << "请问您要读取保存的存档吗？Y/N\n";
    cin >> op; if(op == 'Y') Board.Reload();
    while(!(winner = Board.Judge_Win())) {
        Board.Display(); 
        // DEBUG用！！
        // clock_t startTime = clock();
        // cout << "BestSol:" << Board.getBestSol()<< endl;
        // clock_t endTime = clock();
        // cout << "在" << double(endTime - startTime) / CLOCKS_PER_SEC << "s内生成了BestSol" << endl;
        // DEBUG用！！
        Board.Show_Menu(); cin >> op;
        if(op == 'M') {
            cout << "请按 出发点x坐标 出发点y坐标 目的地x坐标 目的地y坐标 障碍物x坐标 障碍物y坐标 （第x列第y行）的格式输入操作：\n";
            cin >> sx >> sy >> ex >> ey >> bx >> by;
            int ErrorMessage = Board.Move(sy-1, sx-1, ey-1, ex-1, by-1, bx-1); // 面向用户
            if(!ErrorMessage) Board.Next_Turn();
            else{
                cout << "输入Y继续，否则退出游戏:\n"; cin >> op;
                if(op != 'Y') return 0;
            } 
        } else if(op == 'S') {
            Board.Save();
        } else if(op == 'R') {
            Board.Next_Turn();
            Board.Regret(sy-1, sx-1, ey-1, ex-1, by-1, bx-1);
        } else if(op == 'H') {
            Board.Reset_Hint();
            Board.Hint();
        } else if(op == 'E') {
            return 0;
        } else if(op == 'N') {
            Board.Initialize();
        }
    }
    system("pause");
    return 0;
}
