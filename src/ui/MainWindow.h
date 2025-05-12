#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "app/NavigationSystem.h" // 包含 NavigationSystem
#include "MapWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLineEdit;
class QPushButton;
class QSlider;
class QLabel; // <--- 添加这行，用于前向声明 QLabel
class QSpinBox; // <--- 添加这行，用于前向声明 QSpinBox (因为 randomCarsSpinBox 也是指针)

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onShowMapClicked(); // 这个槽函数已声明，但错误信息中提到的是 onShowNearestPointsClicked
    void onShowNearestPointsClicked(); // <--- 新增：声明 onShowNearestPointsClicked
    void onShowAllPointsClicked();   // <--- 新增：声明 onShowAllPointsClicked
    void onFindShortestPathClicked();
    void onFindFastestPathClicked(); 
    void onAddCarClicked();
    void onSimulateTrafficClicked();
    void onZoomSliderChanged(int value); 
    void onCongestionThresholdChanged(int value); 
    void onResetViewClicked(); 
    void onStartSimulationClicked();
    void onStopSimulationClicked();
    void onSimulationStepTimeout();
    void onSimulationSpeedChanged(int value);
    // void onCongestionThresholdChanged(int value); // <--- 移除这个重复的声明 (已在上一轮修复)
    // void onZoomSliderChanged(int value); // <--- 移除这个重复的声明 (已在上一轮修复)

private:
    Ui::MainWindow *ui; // 如果你使用 .ui 文件 (可选)

    // 手动创建UI元素
    QLineEdit *xCoordInput;
    QLineEdit *yCoordInput;
    QPushButton *showPointsButton;
    QPushButton *showAllPointsButton; // 新增按钮

    // 新增：用于路径查找的UI元素
    QLineEdit *startPointInput;
    QLineEdit *endPointInput;
    QPushButton *findPathButton;

    // 新增：地图缩放滑块
    QSlider* zoomSlider; // <--- 新增

    // 新增：车流模拟相关控件
    QLineEdit* startPointIdEdit;
    QLineEdit* endPointIdEdit;
    QPushButton* addCarButton;
    QPushButton* startSimulationButton;
    QPushButton* stopSimulationButton;
    QSlider* simulationSpeedSlider;
    QSlider* congestionThresholdSlider;
    QLabel* simulationTimeLabel;
    QSpinBox* randomCarsSpinBox;
    QPushButton* addRandomCarsButton;
    
    // 新增：模拟计时器
    QTimer* simulationTimer;
    double simulationTimeStep = 0.1; // 默认时间步长
    
    // 新增：创建车流模拟控制面板
    void createTrafficSimulationPanel();
    
    // 新增：添加随机车辆
    void addRandomCars(int count);

    // 新增：手动设置UI的函数声明
    void setupUiManual(); // <--- 添加这一行
    
    NavigationSystem* navSystem;
    MapWidget* mapWidget;
};

#endif // MAINWINDOW_H