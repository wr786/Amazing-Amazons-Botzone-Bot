#include "amazons.h"
#include<ctime> // 仅仅测试效率用！！！！！

// 待实现功能备忘录
// 1. AI
// 2. GUI
// 3. 分离判断可落子点与可放障碍物点（重复调用两次同一个函数即可）
// 4. 改进生成可行解函数的效率

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
        } else if(op == 'H') {
            Board.Reset_Hint();
            Board.Hint();
        } else if(op == 'E') {
            return 0;
        } else if(op == 'N') {
            Board.Reset();
        } else if(op == 'T') { // 测试功能用！！！！！
            // 放大法
            clock_t startTime = clock();
            for(int i=1; i<=10000; i++) {
                Board.Find_Solutions();
            }
            clock_t endTime = clock();
            cout << "在" << double(endTime - startTime) / CLOCKS_PER_SEC << "s内生成了" << 10000 * Board.SolutionList.idx << "种Move方法" << endl;
            system("pause");
            // for(vector<int>::iterator it = SolutionList.begin(); it != SolutionList.end(); it++) {
            //     cout << *it << endl;
            // }
        }
    }
    system("pause");
    return 0;
}
