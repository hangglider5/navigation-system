#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QStatusBar>    // <--- 添加这行
#include <string> // For std::stod
#include <QLineEdit>    // <--- 添加这行
#include <QPushButton>  // <--- 添加这行
#include <QSlider>      // 确保已包含
#include <QTimer>       // 添加这行
#include <QSpinBox>     // 添加这行
#include <random>       // 添加这行

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(nullptr) {

    // 初始化导航系统
    int numPoints = 1000;
    int viewportWidth = 800;
    int viewportHeight = 600;
    navSystem = new NavigationSystem(numPoints, viewportWidth, viewportHeight);

    // 启动初始化过程
    navSystem->initialize();

    // 设置UI
    setupUiManual();
    
    // 设置窗口标题
    setWindowTitle("导航系统");

    // 设置一个合适的初始窗口大小
    resize(850, 800); // <--- 新增：设置窗口初始尺寸
    
    // 创建一个定时器，定期检查导航系统是否初始化完成
    QTimer* initCheckTimer = new QTimer(this);
    connect(initCheckTimer, &QTimer::timeout, [this, initCheckTimer]() {
        if (navSystem->isInitialized()) {
            // 导航系统初始化完成后，设置MapWidget的交通模拟器
            mapWidget->setTrafficSimulator(navSystem->getTrafficSimulator());
            
            // 停止定时器
            initCheckTimer->stop();
            initCheckTimer->deleteLater();
            
            // 显示初始化完成消息
            statusBar()->showMessage("导航系统初始化完成", 3000);
        }
    });
    initCheckTimer->start(500); // 每500毫秒检查一次
}

MainWindow::~MainWindow() {
    delete navSystem;
    // Qt 会自动删除作为子控件添加到布局中的控件，所以不需要手动 delete xCoordInput 等
    // 如果你使用了 .ui 文件，则 delete ui;
}

void MainWindow::setupUiManual() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 坐标输入组
    QGroupBox *coordInputGroup = new QGroupBox("输入坐标");
    QHBoxLayout *coordInputLayout = new QHBoxLayout();
    xCoordInput = new QLineEdit();
    xCoordInput->setPlaceholderText("X 坐标");
    yCoordInput = new QLineEdit();
    yCoordInput->setPlaceholderText("Y 坐标");
    showPointsButton = new QPushButton("显示最近的100个点");
    showAllPointsButton = new QPushButton("显示所有点");

    coordInputLayout->addWidget(new QLabel("X:"));
    coordInputLayout->addWidget(xCoordInput);
    coordInputLayout->addWidget(new QLabel("Y:"));
    coordInputLayout->addWidget(yCoordInput);
    coordInputLayout->addWidget(showPointsButton);
    coordInputLayout->addWidget(showAllPointsButton);
    coordInputGroup->setLayout(coordInputLayout);

    // 路径查找输入组
    QGroupBox *pathInputGroup = new QGroupBox("查找最短路径");
    QHBoxLayout *pathInputLayout = new QHBoxLayout();
    startPointInput = new QLineEdit();
    startPointInput->setPlaceholderText("起点ID");
    endPointInput = new QLineEdit();
    endPointInput->setPlaceholderText("终点ID");
    findPathButton = new QPushButton("查找最短路径");
    QPushButton *findFastestPathButton = new QPushButton("查找最快路径"); // 新增：最快路径按钮

    pathInputLayout->addWidget(new QLabel("起点ID:"));
    pathInputLayout->addWidget(startPointInput);
    pathInputLayout->addWidget(new QLabel("终点ID:"));
    pathInputLayout->addWidget(endPointInput);
    pathInputLayout->addWidget(findPathButton);
    pathInputLayout->addWidget(findFastestPathButton); // 新增：添加最快路径按钮到布局
    pathInputGroup->setLayout(pathInputLayout);

    mapWidget = new MapWidget();
    mapWidget->setMinimumSize(600, 400);

    // 新增：地图控制组 (包含缩放滑块)
    QGroupBox *mapControlGroup = new QGroupBox("地图控制");
    QHBoxLayout *mapControlLayout = new QHBoxLayout();
    zoomSlider = new QSlider(Qt::Horizontal);
    zoomSlider->setMinimum(10); // 对应 0.1x 缩放
    zoomSlider->setMaximum(500); // 对应 5.0x 缩放
    zoomSlider->setValue(100);   // 对应 1.0x 缩放 (默认值)
    zoomSlider->setToolTip("地图缩放");

    mapControlLayout->addWidget(new QLabel("缩放:"));
    mapControlLayout->addWidget(zoomSlider);
    mapControlGroup->setLayout(mapControlLayout);

    mainLayout->addWidget(coordInputGroup);
    mainLayout->addWidget(pathInputGroup);
    mainLayout->addWidget(mapControlGroup); // <--- 添加地图控制组
    mainLayout->addWidget(mapWidget, 1);

    setCentralWidget(centralWidget);

    connect(showPointsButton, &QPushButton::clicked, this, &MainWindow::onShowNearestPointsClicked);
    connect(showAllPointsButton, &QPushButton::clicked, this, &MainWindow::onShowAllPointsClicked);
    connect(findPathButton, &QPushButton::clicked, this, &MainWindow::onFindShortestPathClicked);
    connect(findFastestPathButton, &QPushButton::clicked, this, &MainWindow::onFindFastestPathClicked); // 新增：连接最快路径按钮
    connect(zoomSlider, &QSlider::valueChanged, this, &MainWindow::onZoomSliderChanged); // <--- 连接缩放滑块信号
    
    // 创建车流模拟控制面板
    createTrafficSimulationPanel();
}

void MainWindow::onShowNearestPointsClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }

    bool xOk, yOk;
    double x = xCoordInput->text().toDouble(&xOk);
    double y = yCoordInput->text().toDouble(&yOk);

    if (!xOk || !yOk) {
        QMessageBox::warning(this, "输入错误", "请输入有效的数字坐标。");
        if (mapWidget) { // 修改: mapDisplayWidget -> mapWidget
            mapWidget->clearSpecialPoint(); // 修改: mapDisplayWidget -> mapWidget
            mapWidget->setMapData({}, {});   // 修改: mapDisplayWidget -> mapWidget
        }
        return;
    }

    // 成功获取坐标后，设置特殊标记点
    if (mapWidget) { // 修改: mapDisplayWidget -> mapWidget
        mapWidget->setSpecialPoint(QPointF(x, y)); // 修改: mapDisplayWidget -> mapWidget
    }

    // 调用 NavigationSystem 获取数据
    auto mapData = navSystem->getPointsAndRoadsNear(x, y, 100);

    if (mapData.first.empty() && mapWidget) { // 修改: mapDisplayWidget -> mapWidget
        // 即使没有找到附近的点，我们依然希望显示输入的特殊标记点，
        // 所以这里只提示信息，setMapData 会用空数据更新，但特殊点会保留。
        QMessageBox::information(this, "提示", "在指定坐标附近未找到任何业务点。输入的坐标点已在地图上标记。");
    }

    if (mapWidget) { // 修改: mapDisplayWidget -> mapWidget
        mapWidget->setMapData(mapData.first, mapData.second); // 修改: mapDisplayWidget -> mapWidget
    }
}

void MainWindow::onShowAllPointsClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }

    if (mapWidget) { // 修改: mapDisplayWidget -> mapWidget
        mapWidget->clearSpecialPoint(); // 修改: mapDisplayWidget -> mapWidget
        mapWidget->resetView(); // 调用 resetView // 修改: mapDisplayWidget -> mapWidget
    }

    auto mapData = navSystem->getAllPointsAndRoads();

    if (mapData.first.empty() && mapWidget) { // 修改: mapDisplayWidget -> mapWidget
        QMessageBox::information(this, "提示", "地图中没有点可以显示。");
    }

    if (mapWidget) { // 修改: mapDisplayWidget -> mapWidget
        mapWidget->setMapData(mapData.first, mapData.second); // 修改: mapDisplayWidget -> mapWidget
    }
}

// 新增：处理查找最快路径按钮点击事件的槽函数
void MainWindow::onFindFastestPathClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }

    bool startOk, endOk;
    int startPointId = startPointInput->text().toInt(&startOk);
    int endPointId = endPointInput->text().toInt(&endOk);

    if (!startOk || !endOk) {
        QMessageBox::warning(this, "输入错误", "请输入有效的数字ID作为起点和终点。");
        if (mapWidget) {
            mapWidget->clearSpecialPoint();
            mapWidget->clearShortestPath(); // 复用clearShortestPath来清除路径显示
            mapWidget->clearPathEndpoints(); // 新增：清除路径端点
        }
        return;
    }

    if (startPointId == endPointId) {
        QMessageBox::information(this, "提示", "起点和终点相同。");
        if (mapWidget) {
            mapWidget->clearSpecialPoint();
            mapWidget->clearShortestPath();
            mapWidget->clearPathEndpoints(); // 新增：清除路径端点
            Point* p = navSystem->getPointById(startPointId);
            if (p) {
                 mapWidget->setSpecialPoint(QPointF(p->getX(), p->getY()));
                 mapWidget->setMapData({p}, {});
            } else {
                 mapWidget->setMapData({}, {});
            }
        }
        return;
    }
    
    // 调用 NavigationSystem 获取最快路径数据
    auto pathData = navSystem->getFastestPath(startPointId, endPointId);
    std::vector<Point*> pathPoints = pathData.first;
    std::vector<Road*> pathRoads = pathData.second;

    if (pathPoints.empty() || pathPoints.size() < 2) {
        QMessageBox::information(this, "路径未找到", "无法找到从指定起点到终点的最快路径。");
        if (mapWidget) {
            mapWidget->clearShortestPath(); 
            mapWidget->clearPathEndpoints(); // 新增：清除路径端点
            auto allMapData = navSystem->getAllPointsAndRoads(); // 重新显示整个地图
            mapWidget->setMapData(allMapData.first, allMapData.second);
        }
    } else {
        if (mapWidget) {
            mapWidget->clearSpecialPoint();
            mapWidget->clearShortestPath(); // 清除旧路径
            mapWidget->setShortestPathData(pathPoints, pathRoads); // 复用setShortestPathData显示最快路径
            
            // 新增：设置路径的起点和终点
            Point* startP = pathPoints.front();
            Point* endP = pathPoints.back();
            if (startP && endP) {
                mapWidget->setPathEndpoints(QPointF(startP->getX(), startP->getY()), 
                                           QPointF(endP->getX(), endP->getY()));
            }
            
            auto allMapData = navSystem->getAllPointsAndRoads(); // 确保整个地图可见，路径会高亮
            mapWidget->setMapData(allMapData.first, allMapData.second);
        }
        // 计算并显示预计行驶时间
        // double travelTime = navSystem->pathFinder->calculatePathTravelTime(pathPoints, DEFAULT_C, DEFAULT_THRESHOLD); //  <--- 旧代码
        double travelTime = navSystem->getPathTravelTime(pathPoints); // <--- 修改后的代码
        QMessageBox::information(this, "路径已找到", QString("最快路径已在地图上高亮显示。\n预计行驶时间: %1").arg(travelTime));
    }
}

void MainWindow::onAddCarClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }
    
    bool startOk, endOk;
    int startId = startPointIdEdit->text().toInt(&startOk);
    int endId = endPointIdEdit->text().toInt(&endOk);
    
    if (!startOk || !endOk) {
        QMessageBox::warning(this, "输入错误", "请输入有效的点ID。");
        return;
    }
    
    // 检查点是否存在
    if (!navSystem->getPointById(startId) || !navSystem->getPointById(endId)) {
        QMessageBox::warning(this, "输入错误", "起点或终点ID不存在。");
        return;
    }
    
    // 添加车辆到模拟中
    navSystem->addCarToSimulation(startId, endId);
    
    // 更新显示
    mapWidget->updateTrafficDisplay();
}

void MainWindow::addRandomCars(int count) {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }
    
    // 获取所有点
    auto [allPoints, _] = navSystem->getAllPointsAndRoads();
    if (allPoints.size() < 2) {
        QMessageBox::warning(this, "错误", "地图中点的数量不足。");
        return;
    }
    
    // 随机生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, allPoints.size() - 1);
    
    // 添加随机车辆
    for (int i = 0; i < count; i++) {
        int startIdx = dist(gen);
        int endIdx;
        do {
            endIdx = dist(gen);
        } while (endIdx == startIdx); // 确保起点和终点不同
        
        int startId = allPoints[startIdx]->getId();
        int endId = allPoints[endIdx]->getId();
        
        navSystem->addCarToSimulation(startId, endId);
    }
    
    // 更新显示
    mapWidget->updateTrafficDisplay();
}

void MainWindow::onStartSimulationClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }
    
    // 设置按钮状态
    startSimulationButton->setEnabled(false);
    stopSimulationButton->setEnabled(true);
    
    // 启动计时器
    int interval = 1000 / simulationSpeedSlider->value(); // 毫秒
    simulationTimer->start(interval);
}

void MainWindow::onStopSimulationClicked() {
    // 停止计时器
    simulationTimer->stop();
    
    // 设置按钮状态
    startSimulationButton->setEnabled(true);
    stopSimulationButton->setEnabled(false);
}

void MainWindow::onSimulationStepTimeout() {
    if (!navSystem || !navSystem->isInitialized()) {
        simulationTimer->stop();
        return;
    }
    
    // 模拟一个时间步长
    navSystem->simulateTraffic(simulationTimeStep);
    
    // 更新显示
    mapWidget->updateTrafficDisplay();
    
    // 更新模拟时间标签
    double currentTime = navSystem->getTrafficSimulator()->getCurrentTime();
    simulationTimeLabel->setText(QString("模拟时间: %1").arg(currentTime, 0, 'f', 1));
}

void MainWindow::onSimulationSpeedChanged(int value) {
    // 更新计时器间隔
    if (simulationTimer->isActive()) {
        int interval = 1000 / value; // 毫秒
        simulationTimer->setInterval(interval);
    }
}

void MainWindow::onCongestionThresholdChanged(int value) {
    // 更新拥堵阈值
    double threshold = value / 100.0;
    navSystem->setTrafficThreshold(threshold);
    mapWidget->setCongestionThreshold(threshold);
}

// 新增：处理缩放滑块变化的槽函数
void MainWindow::onZoomSliderChanged(int value) {
    if (mapWidget) {
        double zoomFactor = static_cast<double>(value) / 100.0; // 将滑块值转换为缩放因子
        mapWidget->setZoomFactor(zoomFactor);
    }
}

// 实现 onShowMapClicked 函数
void MainWindow::onShowMapClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }

    bool xOk, yOk;
    double x = xCoordInput->text().toDouble(&xOk);
    double y = yCoordInput->text().toDouble(&yOk);

    if (!xOk || !yOk) {
        QMessageBox::warning(this, "输入错误", "请输入有效的数字坐标。");
        if (mapWidget) {
            mapWidget->clearSpecialPoint();
            mapWidget->setMapData({}, {});
        }
        return;
    }

    // 显示地图
    navSystem->showMapAroundLocation(x, y);
    
    // 设置特殊标记点
    if (mapWidget) {
        mapWidget->setSpecialPoint(QPointF(x, y));
        auto mapData = navSystem->getPointsAndRoadsNear(x, y, 100);
        mapWidget->setMapData(mapData.first, mapData.second);
    }
}

// 实现 onResetViewClicked 函数
void MainWindow::onResetViewClicked() {
    if (mapWidget) {
        mapWidget->resetView();
        mapWidget->update();
    }
}

// 实现 onSimulateTrafficClicked 函数
void MainWindow::onSimulateTrafficClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }
    
    // 模拟一个时间步长的交通
    navSystem->simulateTraffic(simulationTimeStep);
    
    // 更新地图显示
    if (mapWidget) {
        mapWidget->updateTrafficDisplay();
    }
}

// 实现 onFindShortestPathClicked 函数
void MainWindow::onFindShortestPathClicked() {
    if (!navSystem || !navSystem->isInitialized()) {
        QMessageBox::warning(this, "错误", "导航系统尚未初始化完毕。请稍后再试。");
        return;
    }

    bool startOk, endOk;
    int startPointId = startPointInput->text().toInt(&startOk);
    int endPointId = endPointInput->text().toInt(&endOk);

    if (!startOk || !endOk) {
        QMessageBox::warning(this, "输入错误", "请输入有效的数字ID作为起点和终点。");
        if (mapWidget) {
            mapWidget->clearSpecialPoint();
            mapWidget->clearShortestPath();
            mapWidget->clearPathEndpoints(); // 新增：清除路径端点
        }
        return;
    }

    if (startPointId == endPointId) {
        QMessageBox::information(this, "提示", "起点和终点相同。");
        if (mapWidget) {
            mapWidget->clearSpecialPoint();
            mapWidget->clearShortestPath();
            mapWidget->clearPathEndpoints(); // 新增：清除路径端点
            Point* p = navSystem->getPointById(startPointId);
            if (p) {
                 mapWidget->setSpecialPoint(QPointF(p->getX(), p->getY()));
                 mapWidget->setMapData({p}, {});
            } else {
                 mapWidget->setMapData({}, {});
            }
        }
        return;
    }
    
    // 调用 NavigationSystem 获取最短路径数据
    auto pathData = navSystem->getShortestPath(startPointId, endPointId);
    std::vector<Point*> pathPoints = pathData.first;
    std::vector<Road*> pathRoads = pathData.second;

    if (pathPoints.empty() || pathPoints.size() < 2) {
        QMessageBox::information(this, "路径未找到", "无法找到从指定起点到终点的最短路径。");
        if (mapWidget) {
            mapWidget->clearShortestPath(); 
            mapWidget->clearPathEndpoints(); // 新增：清除路径端点
            auto allMapData = navSystem->getAllPointsAndRoads(); // 重新显示整个地图
            mapWidget->setMapData(allMapData.first, allMapData.second);
        }
    } else {
        if (mapWidget) {
            mapWidget->clearSpecialPoint();
            mapWidget->clearShortestPath(); // 清除旧路径
            mapWidget->setShortestPathData(pathPoints, pathRoads);
            
            // 新增：设置路径的起点和终点
            Point* startP = pathPoints.front();
            Point* endP = pathPoints.back();
            if (startP && endP) {
                mapWidget->setPathEndpoints(QPointF(startP->getX(), startP->getY()), 
                                           QPointF(endP->getX(), endP->getY()));
            }
            
            auto allMapData = navSystem->getAllPointsAndRoads(); // 确保整个地图可见，路径会高亮
            mapWidget->setMapData(allMapData.first, allMapData.second);
        }
        QMessageBox::information(this, "路径已找到", "最短路径已在地图上高亮显示。");
    }
}

// 实现 createTrafficSimulationPanel 函数
void MainWindow::createTrafficSimulationPanel() {
    // 创建车流模拟控制面板
    QGroupBox *trafficSimGroup = new QGroupBox("车流模拟");
    QVBoxLayout *trafficSimLayout = new QVBoxLayout();
    
    // 添加车辆控件
    QHBoxLayout *addCarLayout = new QHBoxLayout();
    startPointIdEdit = new QLineEdit();
    startPointIdEdit->setPlaceholderText("起点ID");
    endPointIdEdit = new QLineEdit();
    endPointIdEdit->setPlaceholderText("终点ID");
    addCarButton = new QPushButton("添加车辆");
    
    addCarLayout->addWidget(new QLabel("起点ID:"));
    addCarLayout->addWidget(startPointIdEdit);
    addCarLayout->addWidget(new QLabel("终点ID:"));
    addCarLayout->addWidget(endPointIdEdit);
    addCarLayout->addWidget(addCarButton);
    
    // 随机车辆控件
    QHBoxLayout *randomCarsLayout = new QHBoxLayout();
    randomCarsSpinBox = new QSpinBox();
    randomCarsSpinBox->setMinimum(1);
    randomCarsSpinBox->setMaximum(100);
    randomCarsSpinBox->setValue(10);
    addRandomCarsButton = new QPushButton("添加随机车辆");
    
    randomCarsLayout->addWidget(new QLabel("数量:"));
    randomCarsLayout->addWidget(randomCarsSpinBox);
    randomCarsLayout->addWidget(addRandomCarsButton);
    
    // 模拟控制控件
    QHBoxLayout *simControlLayout = new QHBoxLayout();
    startSimulationButton = new QPushButton("开始模拟");
    stopSimulationButton = new QPushButton("停止模拟");
    stopSimulationButton->setEnabled(false);
    
    simControlLayout->addWidget(startSimulationButton);
    simControlLayout->addWidget(stopSimulationButton);
    
    // 模拟速度控件
    QHBoxLayout *simSpeedLayout = new QHBoxLayout();
    simulationSpeedSlider = new QSlider(Qt::Horizontal);
    simulationSpeedSlider->setMinimum(1);
    simulationSpeedSlider->setMaximum(20);
    simulationSpeedSlider->setValue(5);
    simulationSpeedSlider->setToolTip("模拟速度");
    
    simSpeedLayout->addWidget(new QLabel("模拟速度:"));
    simSpeedLayout->addWidget(simulationSpeedSlider);
    
    // 拥堵阈值控件
    QHBoxLayout *congestionLayout = new QHBoxLayout();
    congestionThresholdSlider = new QSlider(Qt::Horizontal);
    congestionThresholdSlider->setMinimum(0);
    congestionThresholdSlider->setMaximum(100);
    congestionThresholdSlider->setValue(50);
    congestionThresholdSlider->setToolTip("拥堵阈值");
    
    congestionLayout->addWidget(new QLabel("拥堵阈值:"));
    congestionLayout->addWidget(congestionThresholdSlider);
    
    // 模拟时间标签
    simulationTimeLabel = new QLabel("模拟时间: 0.0");
    
    // 添加所有控件到布局
    trafficSimLayout->addLayout(addCarLayout);
    trafficSimLayout->addLayout(randomCarsLayout);
    trafficSimLayout->addLayout(simControlLayout);
    trafficSimLayout->addLayout(simSpeedLayout);
    trafficSimLayout->addLayout(congestionLayout);
    trafficSimLayout->addWidget(simulationTimeLabel);
    
    trafficSimGroup->setLayout(trafficSimLayout);
    
    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->insertWidget(mainLayout->count() - 1, trafficSimGroup);
    }
    
    // 创建模拟计时器
    simulationTimer = new QTimer(this);
    
    // 连接信号和槽
    connect(addCarButton, &QPushButton::clicked, this, &MainWindow::onAddCarClicked);
    connect(addRandomCarsButton, &QPushButton::clicked, [this]() {
        addRandomCars(randomCarsSpinBox->value());
    });
    connect(startSimulationButton, &QPushButton::clicked, this, &MainWindow::onStartSimulationClicked);
    connect(stopSimulationButton, &QPushButton::clicked, this, &MainWindow::onStopSimulationClicked);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::onSimulationStepTimeout);
    connect(simulationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::onSimulationSpeedChanged);
    connect(congestionThresholdSlider, &QSlider::valueChanged, this, &MainWindow::onCongestionThresholdChanged);
}
