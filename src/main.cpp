// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// main.cpp - Qt6 GUI 应用入口
//
// 二维下料问题求解器图形界面
// 基于 CS-2D-BP-Arc 分支定价算法

#include <QApplication>
#include <QStyleFactory>
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 应用元数据
    QApplication::setApplicationName("CS-2D-GUI");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("YM-Code");

    // 使用 Fusion 样式保持一致外观
    app.setStyle(QStyleFactory::create("Fusion"));

    // 创建并显示主窗口
    MainWindow window;
    window.show();

    return app.exec();
}
