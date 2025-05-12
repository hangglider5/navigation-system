#include "../app/NavigationSystem.h"
#include <iostream>
#include <limits>
#include <thread>
#include <chrono>
#include <atomic>
#include <QApplication>
#include "ui/MainWindow.h" // 确保路径正确

// 显示菜单
void showMenu() {
    std::cout << "\n===== 导航系统菜单 =====" << std::endl;
    std::cout << "1. 显示地图信息" << std::endl;
    std::cout << "2. 查看指定位置附近的地图" << std::endl;
    std::cout << "3. 计算两点间的最短路径" << std::endl;
    std::cout << "4. 计算两点间的最快路径（考虑路况）" << std::endl;
    std::cout << "5. 添加车辆到模拟中" << std::endl;
    std::cout << "6. 模拟交通流量" << std::endl;
    std::cout << "7. 缩放地图" << std::endl;
    std::cout << "8. 平移地图" << std::endl;
    std::cout << "0. 退出" << std::endl;
    std::cout << "请输入选项: ";
}

// 清除输入缓冲区
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


// 这是用于 Qt 应用程序的 main 函数，应该保留
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
