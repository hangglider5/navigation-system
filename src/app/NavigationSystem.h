#ifndef NAVIGATION_SYSTEM_H
#define NAVIGATION_SYSTEM_H

#include "../core/Map.h"
#include "../algorithms/MapGenerator.h"
#include "../algorithms/PathFinder.h"
#include "../algorithms/TrafficSimulator.h"
#include "../ui/MapRenderer.h"
#include <vector>
#include <utility> // For std::pair
#include "core/Point.h"
#include "core/Road.h"

class NavigationSystem {
private:
    Map* map;
    MapGenerator* mapGenerator;
    PathFinder* pathFinder;
    TrafficSimulator* trafficSimulator;
    MapRenderer* mapRenderer;
    bool initialized = false; // 添加一个初始化状态标志
public:
    NavigationSystem(int numPoints, int viewportWidth, int viewportHeight);
    ~NavigationSystem();
    
    // 初始化系统
    void initialize(); // 确保有这个声明
    bool isInitialized() const; // 添加这个方法

    // 新方法：获取指定坐标附近的点和相关联的边
    std::pair<std::vector<Point*>, std::vector<Road*>> getPointsAndRoadsNear(double x, double y, int count);
    
    // 新方法：获取地图中的所有点和道路
    std::pair<std::vector<Point*>, std::vector<Road*>> getAllPointsAndRoads();
    
    // 通过ID获取点
    Point* getPointById(int pointId);
    
    // 获取两点间最短路径的点和边
    std::pair<std::vector<Point*>, std::vector<Road*>> getShortestPath(int startPointId, int endPointId);
    
    // 新增：获取两点间最快路径的点和边 (供UI调用)
    std::pair<std::vector<Point*>, std::vector<Road*>> getFastestPath(int startPointId, int endPointId);

    // 显示指定位置附近的地图
    void showMapAroundLocation(double x, double y);
    
    // 新增：显示两点间的最短路径 (修复报错)
    void showShortestPath(int startPointId, int endPointId);

    // 新增：显示两点间的最快路径
    void showFastestPath(int startPointId, int endPointId);

    // 计算两点间的最短路径
    void calculateShortestPath(int startPointId, int endPointId);
    
    // 计算两点间的最快路径（考虑路况）
    void calculateFastestPath(int startPointId, int endPointId);
    
    // 添加车辆到模拟中
    void addCarToSimulation(int startPointId, int endPointId);
    
    // 模拟交通流量
    void simulateTraffic(double timeStep);

    // 新增：地图缩放功能声明
    void zoomMap(double factor);
    
    // 新增：地图平移功能声明
    void panMap(double deltaX, double deltaY); // <--- 添加这一行
    
    // 新增：获取交通模拟器
    TrafficSimulator* getTrafficSimulator() const { return trafficSimulator; }
    PathFinder* getPathFinder() const { return pathFinder; } // <--- 添加这一行，或者创建一个专门计算时间的方法
    
    // 新增：设置交通拥堵阈值
    void setTrafficThreshold(double threshold);
    
    // 计算给定路径的行驶时间
    double getPathTravelTime(const std::vector<Point*>& pathPoints) const;
};

#endif // NAVIGATION_SYSTEM_H