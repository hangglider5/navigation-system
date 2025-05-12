#include "TrafficSimulator.h"
#include "../core/Point.h"
#include "../core/Road.h"
#include "PathFinder.h"
#include <algorithm>

#include <memory> // 添加智能指针头文件

TrafficSimulator::TrafficSimulator(Map* map, double c, double threshold)
    : map(map), currentTime(0.0), c(c), threshold(threshold) {
}

TrafficSimulator::~TrafficSimulator() {
    // 释放所有车辆的内存
    for (auto car : cars) {
        delete car;
    }
}

void TrafficSimulator::addCar(int startPointId, int endPointId) {
    // 创建路径规划器
    PathFinder pathFinder(map);
    
    // 计算从起点到终点的最短路径
    std::vector<Point*> path = pathFinder.findShortestPath(startPointId, endPointId);
    
    // 如果找不到路径，直接返回
    if (path.size() < 2) {
        return;
    }
    
    // 创建新车
    Car* car = new Car();
    car->id = cars.size(); // 使用当前车辆数量作为ID
    car->path = path;
    car->currentRoadIndex = 0;
    car->entryTime = currentTime;
    
    // 更新第一条道路的车流量
    Road* firstRoad = map->getRoadBetweenPoints(path[0]->getId(), path[1]->getId());
    if (firstRoad) {
        firstRoad->setCurrentCars(firstRoad->getCurrentCars() + 1);
    }
    
    // 添加车辆到模拟中
    cars.push_back(car);
}

void TrafficSimulator::simulateTimeStep(double timeStep) {
    // 更新当前时间
    currentTime += timeStep;
    
    // 遍历所有车辆
    for (auto it = cars.begin(); it != cars.end(); ) {
        Car* car = *it;
        
        // 如果车辆已经到达终点，移除它
        if (car->currentRoadIndex >= car->path.size() - 1) {
            delete car;
            it = cars.erase(it);
            continue;
        }
        
        // 获取当前道路
        int startId = car->path[car->currentRoadIndex]->getId();
        int endId = car->path[car->currentRoadIndex + 1]->getId();
        Road* currentRoad = map->getRoadBetweenPoints(startId, endId);
        
        if (!currentRoad) {
            // 如果找不到道路，跳过这辆车
            ++it;
            continue;
        }
        
        // 计算通过当前道路所需的时间
        double travelTime = currentRoad->getTravelTime(c, threshold);
        
        // 如果已经通过当前道路
        if (currentTime - car->entryTime >= travelTime) {
            // 减少当前道路的车流量
            currentRoad->setCurrentCars(currentRoad->getCurrentCars() - 1);
            
            // 移动到下一条道路
            car->currentRoadIndex++;
            car->entryTime = currentTime;
            
            // 如果还没到达终点，增加下一条道路的车流量
            if (car->currentRoadIndex < car->path.size() - 1) {
                int nextStartId = car->path[car->currentRoadIndex]->getId();
                int nextEndId = car->path[car->currentRoadIndex + 1]->getId();
                Road* nextRoad = map->getRoadBetweenPoints(nextStartId, nextEndId);
                
                if (nextRoad) {
                    nextRoad->setCurrentCars(nextRoad->getCurrentCars() + 1);
                }
            }
        }
        
        ++it;
    }
}

double TrafficSimulator::getCurrentTime() const {
    return currentTime;
}

int TrafficSimulator::getCurrentTrafficOnRoad(int roadId) const {
    Road* road = map->getRoadById(roadId);
    if (road) {
        return road->getCurrentCars();
    }
    return 0;
}

double TrafficSimulator::getCongestionLevel(int roadId) const {
    Road* road = map->getRoadById(roadId);
    if (road && road->getCapacity() > 0) {
        return static_cast<double>(road->getCurrentCars()) / road->getCapacity();
    }
    return 0.0;
}

std::vector<std::pair<int, std::shared_ptr<Point>>> TrafficSimulator::getAllCarPositions() const {
    std::vector<std::pair<int, std::shared_ptr<Point>>> positions;
    
    for (auto car : cars) {
        // 如果车辆在道路上
        if (car->currentRoadIndex < car->path.size() - 1) {
            // 获取当前道路
            int startId = car->path[car->currentRoadIndex]->getId();
            int endId = car->path[car->currentRoadIndex + 1]->getId();
            Road* road = map->getRoadBetweenPoints(startId, endId);
            
            if (road) {
                // 计算车辆在道路上的位置
                double travelTime = road->getTravelTime(c, threshold);
                double timeOnRoad = currentTime - car->entryTime;
                double progress = std::min(1.0, timeOnRoad / travelTime);
                
                // 获取起点和终点
                Point* start = car->path[car->currentRoadIndex];
                Point* end = car->path[car->currentRoadIndex + 1];
                
                // 计算车辆的当前位置（线性插值）
                double x = start->getX() + progress * (end->getX() - start->getX());
                double y = start->getY() + progress * (end->getY() - start->getY());
                
                // 使用智能指针创建临时点表示车辆位置
                std::shared_ptr<Point> position = std::make_shared<Point>(-1, x, y);
                positions.push_back(std::make_pair(car->id, position));
            }
        }
    }
    
    return positions;
}

void TrafficSimulator::setThreshold(double newThreshold) {
    threshold = newThreshold;
}