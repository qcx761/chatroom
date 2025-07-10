#include"account.hpp"
using namespace std;

void waiting() {
    std::cout << "按 Enter 键继续...";
    std::cin.get();  // 等待按回车
}

void flushInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void main_menu_ui(int sock) {
    int n;
    while (1) {
        system("clear"); // 清屏
        show_main_menu();

        std::cout << "请输入你的选项：";
        if (!(std::cin >> n)) {
            flushInput();
            std::cout << "无效的输入，请输入数字。" << std::endl;
            waiting();
            continue;
        }

        switch (n) {
        case 1:
            log_in(sock);

            //waiting();

            break;
        case 2:
            sign_up(sock);

            //waiting();

            break;
        case 3:
            exit(0);
        default:
            std::cout << "无效数字" << std::endl;
            
            waiting();
            break;
        }
    }
}


void log_in(int sock){
    cout<<"111"<<endl;
    //直接刷新了怎么停留




}

void sign_up(int sock){
cout<<"222"<<endl;
}