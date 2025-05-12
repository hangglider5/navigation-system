#include "NavigationSystem.h"
#include <iostream>
#include <thread> // 添加线程头文件
#include <unordered_set> // 需要包含这个头文件

// 常量定义
const double DEFAULT_C = 0.1;           // 道路通行时间计算中的常数c
const double DEFAULT_THRESHOLD = 0.7;    // 拥堵判断阈值
const double DEFAULT_MAX_ROAD_DISTANCE = 100.0; // 连接点的最大距离

NavigationSystem::NavigationSystem(int numPoints, int viewportWidth, int viewportHeight) {
    // 初始化地图生成器
    mapGenerator = new MapGenerator(numPoints, 1000.0, 1000.0, DEFAULT_MAX_ROAD_DISTANCE);

    // 地图、路径查找器和交通模拟器将在initialize()中创建
    map = nullptr; // 确保 map 初始为 nullptr
    pathFinder = nullptr;
    trafficSimulator = nullptr;
    initialized = false; // 确保 initialized 初始为 false

    // 初始化地图渲染器
    mapRenderer = new MapRenderer(nullptr, nullptr, viewportWidth, viewportHeight);
}

NavigationSystem::~NavigationSystem() {
    // 释放所有分配的内存
    delete mapGenerator;
    delete map;
    delete pathFinder;
    delete trafficSimulator;
    delete mapRenderer;
}

void NavigationSystem::initialize() {
    // initialized 默认为 false (在构造函数中设置)
    // map, pathFinder, trafficSimulator 默认为 nullptr (在构造函数中设置)

    std::cout << "启动导航系统后台初始化线程..." << std::endl;

    std::thread initWorkerThread([this]() {
        std::cout << "[后台线程] 开始生成地图..." << std::endl;
        this->map = this->mapGenerator->generateMap(); // mapGenerator 应返回一个新创建并填充的 Map 对象

        if (!this->map) {
            std::cerr << "[后台线程] 错误：地图生成失败！" << std::endl;
            // initialized 将保持 false，系统将不会标记为可用
            return;
        }

        std::cout << "[后台线程] 地图生成完毕。开始构建KD树..." << std::endl;
        this->map->rebuildKDTree();

        std::cout << "[后台线程] KD树构建完毕。开始创建路径查找器..." << std::endl;
        this->pathFinder = new PathFinder(this->map);

        std::cout << "[后台线程] 路径查找器创建完毕。开始创建交通模拟器..." << std::endl;
        this->trafficSimulator = new TrafficSimulator(this->map, DEFAULT_C, DEFAULT_THRESHOLD);

        std::cout << "[后台线程] 交通模拟器创建完毕。设置地图渲染器..." << std::endl;
        if (this->mapRenderer) {
            this->mapRenderer->setMap(this->map);
            this->mapRenderer->setTrafficSimulator(this->trafficSimulator);
        }

        this->initialized = true; // 关键：仅在所有后台操作完成后设置
        std::cout << "[后台线程] 导航系统初始化完成！" << std::endl;
        // 如果 NavigationSystem 是 QObject，可以在这里 emit一个信号通知初始化完成
        // emit initializationCompleted();
    });

    // 分离线程，让它在后台运行，不阻塞UI
    initWorkerThread.detach();

    // 主线程不应再执行地图生成或设置 initialized = true
    // 下面的冗余/冲突代码已被移除
    std::cout << "导航系统初始化过程已在后台启动。" << std::endl;
}

bool NavigationSystem::isInitialized() const {
    return initialized;
}

std::pair<std::vector<Point*>, std::vector<Road*>> NavigationSystem::getPointsAndRoadsNear(double x, double y, int count) {
    if (!initialized || !map) { // 确保检查 initialized 和 map
        if (!initialized) {
            std::cout << "getPointsAndRoadsNear: 系统尚未初始化。" << std::endl;
        }
        if (!map) {
            std::cout << "getPointsAndRoadsNear: 地图对象为空。" << std::endl;
        }
        return {{}, {}};
    }
    std::vector<Point*> nearPoints = map->getNearestPoints(x, y, count);
    std::vector<Road*> relevantRoads;
    std::unordered_set<int> roadIds; // 用于避免重复添加道路

    if (nearPoints.empty()) {
        // std::cout << "getPointsAndRoadsNear: map->getNearestPoints 未找到点。" << std::endl; // 可选的调试输出
        return {nearPoints, relevantRoads};
    }

    for (Point* p1 : nearPoints) {
        if (!p1) continue;
        std::vector<Road*> roadsFromP1 = map->getRoadsFromPoint(p1->getId());
        for (Road* road : roadsFromP1) {
            if (!road || roadIds.count(road->getId())) {
                continue; // 道路为空或已添加
            }

            Point* p2 = (road->getStartPoint()->getId() == p1->getId()) ? road->getEndPoint() : road->getStartPoint();
            if (!p2) continue;

            // 检查 p2 是否也在 nearPoints 列表中
            bool p2_is_near = false;
            for (Point* np : nearPoints) {
                if (np && np->getId() == p2->getId()) {
                    p2_is_near = true;
                    break;
                }
            }

            if (p2_is_near) {
                relevantRoads.push_back(road);
                roadIds.insert(road->getId());
            }
        }
    }
    return {nearPoints, relevantRoads};
}

// 新增：获取地图中的所有点和道路
std::pair<std::vector<Point*>, std::vector<Road*>> NavigationSystem::getAllPointsAndRoads() {
    if (!initialized || !map) {
        if (!initialized) {
            std::cout << "getAllPointsAndRoads: 系统尚未初始化。" << std::endl;
        }
        if (!map) {
            std::cout << "getAllPointsAndRoads: 地图对象为空。" << std::endl;
        }
        return {{}, {}};
    }
    return {map->getAllPoints(), map->getAllRoads()};
}

// 新增：通过ID获取点
Point* NavigationSystem::getPointById(int pointId) {
    if (!initialized || !map) {
        return nullptr;
    }
    return map->getPointById(pointId);
}

// 新增：获取两点间最短路径的点和边
std::pair<std::vector<Point*>, std::vector<Road*>> NavigationSystem::getShortestPath(int startPointId, int endPointId) {
    if (!initialized || !pathFinder || !map) {
        std::cout << "getShortestPath: 系统或路径查找器未初始化。" << std::endl;
        return {{}, {}};
    }

    // 检查点是否存在
    Point* startP = map->getPointById(startPointId);
    Point* endP = map->getPointById(endPointId);

    if (!startP || !endP) {
        std::cout << "错误：起点或终点ID无效！" << std::endl;
        return {{}, {}};
    }

    std::vector<Point*> pathPoints = pathFinder->findShortestPath(startPointId, endPointId);

    if (pathPoints.size() < 2) { // 路径至少需要两个点
        // std::cout << "无法找到从点 " << startPointId << " 到点 " << endPointId << " 的路径！" << std::endl;
        return {{}, {}}; // 返回空数据
    }

    std::vector<Road*> pathRoads = pathFinder->getRoadsInPath(pathPoints);

    return {pathPoints, pathRoads};
}

// 新增：实现获取最快路径数据的方法
std::pair<std::vector<Point*>, std::vector<Road*>> NavigationSystem::getFastestPath(int startPointId, int endPointId) {
    if (!initialized || !pathFinder || !map || !trafficSimulator) {
        std::cout << "getFastestPath: 系统、路径查找器、地图或交通模拟器未初始化。" << std::endl;
        return {{}, {}};
    }

    // 检查点是否存在
    Point* startP = map->getPointById(startPointId);
    Point* endP = map->getPointById(endPointId);

    if (!startP || !endP) {
        std::cout << "错误：起点或终点ID无效！" << std::endl;
        return {{}, {}};
    }

    // 使用 PathFinder 计算最快路径的点
    // 注意：这里的 DEFAULT_C 和 DEFAULT_THRESHOLD 是在 NavigationSystem.cpp 顶部定义的常量
    std::vector<Point*> pathPoints = pathFinder->findFastestPath(startPointId, endPointId, DEFAULT_C, DEFAULT_THRESHOLD);

    if (pathPoints.size() < 2) { // 路径至少需要两个点
        // std::cout << "无法找到从点 " << startPointId << " 到点 " << endPointId << " 的最快路径！" << std::endl;
        return {{}, {}}; // 返回空数据
    }

    // 获取路径上的道路
    std::vector<Road*> pathRoads = pathFinder->getRoadsInPath(pathPoints);

    return {pathPoints, pathRoads};
}


void NavigationSystem::showMapAroundLocation(double x, double y) {
    if (!initialized) {
        std::cout << "System not initialized." << std::endl;
        return;
    }
    // 获取附近的点
    auto data = getPointsAndRoadsNear(x, y, 100); // 使用新方法

    // (可选) 如果你还想在控制台输出信息：
    std::cout << "查询坐标 (" << x << ", " << y << ") 附近的点和路:" << std::endl;
    std::cout << "找到 " << data.first.size() << " 个点." << std::endl;
    for(Point* p : data.first) {
        std::cout << "  点 " << p->getId() << " (" << p->getX() << ", " << p->getY() << ")" << std::endl;
    }
    std::cout << "找到 " << data.second.size() << " 条相关道路." << std::endl;
    for(Road* r : data.second) {
        std::cout << "  路 " << r->getId() << " 从 " << r->getStartPoint()->getId() << " 到 " << r->getEndPoint()->getId() << std::endl;
    }

    // 原 mapRenderer->renderNearLocation(x, y, nearPoints); 调用不再是GUI的主要部分
    // 但 MapRenderer 类本身可能还有其他用途或可以被移除/重构
}

void NavigationSystem::showShortestPath(int startPointId, int endPointId) {
    // 检查点是否存在
    if (!map->getPointById(startPointId) || !map->getPointById(endPointId)) {
        std::cout << "错误：起点或终点ID无效！" << std::endl;
        return;
    }

    // 计算最短路径
    std::vector<Point*> path = pathFinder->findShortestPath(startPointId, endPointId);

    // 如果找不到路径
    if (path.size() < 2) {
        std::cout << "无法找到从点 " << startPointId << " 到点 " << endPointId << " 的路径！" << std::endl;
        return;
    }

    // 计算路径长度
    double pathLength = pathFinder->calculatePathLength(path);

    // 显示路径信息
    std::cout << "从点 " << startPointId << " 到点 " << endPointId
              << " 的最短路径长度为: " << pathLength << std::endl;

    // 高亮显示路径
    mapRenderer->highlightPath(path);
}

void NavigationSystem::showFastestPath(int startPointId, int endPointId) {
    // 检查点是否存在
    if (!map->getPointById(startPointId) || !map->getPointById(endPointId)) {
        std::cout << "错误：起点或终点ID无效！" << std::endl;
        return;
    }

    // 计算最快路径（考虑路况）
    std::vector<Point*> path = pathFinder->findFastestPath(startPointId, endPointId, DEFAULT_C, DEFAULT_THRESHOLD);

    // 如果找不到路径
    if (path.size() < 2) {
        std::cout << "无法找到从点 " << startPointId << " 到点 " << endPointId << " 的路径！" << std::endl;
        return;
    }

    // 计算路径长度和行驶时间
    double pathLength = pathFinder->calculatePathLength(path);
    double travelTime = pathFinder->calculatePathTravelTime(path, DEFAULT_C, DEFAULT_THRESHOLD);

    // 显示路径信息
    std::cout << "从点 " << startPointId << " 到点 " << endPointId
              << " 的最快路径：" << std::endl;
    std::cout << "路径长度: " << pathLength << std::endl;
    std::cout << "预计行驶时间: " << travelTime << std::endl;

    // 高亮显示路径
    mapRenderer->highlightPath(path);
}

void NavigationSystem::simulateTraffic(double timeStep) {
    if (!initialized || !trafficSimulator) {
        return;
    }
    
    // 调用交通模拟器的模拟方法
    trafficSimulator->simulateTimeStep(timeStep);
}

void NavigationSystem::setTrafficThreshold(double threshold) {
    if (!initialized || !trafficSimulator) {
        return;
    }
    
    // 设置交通模拟器的拥堵阈值
    trafficSimulator->setThreshold(threshold);
}

void NavigationSystem::zoomMap(double factor) {
    // 缩放地图
    mapRenderer->zoom(factor);
}

void NavigationSystem::panMap(double deltaX, double deltaY) {
    // 平移地图
    mapRenderer->pan(deltaX, deltaY);
}

void NavigationSystem::addCarToSimulation(int startPointId, int endPointId) {
    // 检查点是否存在
    if (!map->getPointById(startPointId) || !map->getPointById(endPointId)) {
        std::cout << "错误：起点或终点ID无效！" << std::endl;
        return;
    }

    // 添加车辆到模拟中
    trafficSimulator->addCar(startPointId, endPointId);
    std::cout << "已添加一辆从点 " << startPointId << " 到点 " << endPointId << " 的车辆到模拟中。" << std::endl;
}

// 删除这里的第二个 setTrafficThreshold 函数定义
// void NavigationSystem::setTrafficThreshold(double threshold) {
//     if (trafficSimulator) {
//         trafficSimulator->setThreshold(threshold);
//         std::cout << "交通拥堵阈值已设置为: " << threshold << std::endl;
//     } else {
//         std::cout << "错误: 交通模拟器未初始化，无法设置拥堵阈值。" << std::endl;
//     }
// }
double NavigationSystem::getPathTravelTime(const std::vector<Point*>& pathPoints) const {
    if (!pathFinder || pathPoints.empty()) {
        return 0.0;
    }
    return pathFinder->calculatePathTravelTime(pathPoints, DEFAULT_C, DEFAULT_THRESHOLD);
}
