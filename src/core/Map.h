#ifndef MAP_H
#define MAP_H

#include <vector>
#include <unordered_map>
#include "Point.h"
#include "Road.h"
#include "../algorithms/KDTree.h"

class Map {
private:
    std::vector<Point*> points;
    std::vector<Road*> roads;
    std::unordered_map<int, std::vector<Road*>> adjacencyList; // 邻接表表示图
    KDTree* kdTree; // KD树用于快速查找最近点
    
public:
    Map();
    ~Map();
    
    // 添加点和道路
    void addPoint(Point* point);
    void addRoad(Road* road);
    
    // 获取点和道路
    Point* getPointById(int id) const;
    Road* getRoadById(int id) const;
    std::vector<Point*> getAllPoints() const;
    std::vector<Road*> getAllRoads() const;
    
    // 获取与某点相连的所有道路
    std::vector<Road*> getRoadsFromPoint(int pointId) const;
    
    // 获取两点之间的道路（如果存在）
    Road* getRoadBetweenPoints(int startId, int endId) const;
    
    // 获取指定坐标附近的点
    std::vector<Point*> getNearestPoints(double x, double y, int count) const;
    
    // 获取与指定点相连的所有点
    std::vector<Point*> getAdjacentPoints(int pointId) const;
    
    // 检查图是否连通
    bool isConnected() const;
    
    // 重建KD树
    void rebuildKDTree();
    
    // 添加预分配内存的方法
    void reserveCapacity(size_t numPoints, size_t numRoads) {
        points.reserve(numPoints);
        roads.reserve(numRoads);
        // 为邻接表预分配空间
        adjacencyList.reserve(numPoints);
    }
};

#endif // MAP_H