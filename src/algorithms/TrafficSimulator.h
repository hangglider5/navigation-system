#ifndef TRAFFIC_SIMULATOR_H
#define TRAFFIC_SIMULATOR_H

#include "../core/Map.h"
#include <vector>
#include <queue>
#include <memory> // 添加智能指针头文件

// 表示一辆车
struct Car {
    int id;
    std::vector<Point*> path;
    int currentRoadIndex;
    double entryTime; // 进入当前道路的时间
};

class TrafficSimulator {
private:
    Map* map;
    std::vector<Car*> cars;
    double currentTime;
    double c; // 常数c
    double threshold; // f(x)函数的阈值
    
public:
    TrafficSimulator(Map* map, double c, double threshold);
    ~TrafficSimulator();
    
    // 添加一辆新车，指定起点和终点
    void addCar(int startPointId, int endPointId);
    
    // 模拟时间前进
    void simulateTimeStep(double timeStep);
    
    // 获取当前时间
    double getCurrentTime() const;
    
    // 获取道路当前的车流量
    int getCurrentTrafficOnRoad(int roadId) const;
    
    // 获取道路当前的拥堵程度（n/v的值）
    double getCongestionLevel(int roadId) const;
    
    // 获取所有车辆的当前位置（使用智能指针）
    std::vector<std::pair<int, std::shared_ptr<Point>>> getAllCarPositions() const;
    
    // 新增：设置阈值
    void setThreshold(double newThreshold);
};

#endif // TRAFFIC_SIMULATOR_H