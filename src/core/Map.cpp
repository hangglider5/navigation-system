#include "Map.h"
#include <algorithm>
#include <queue>
#include <unordered_set>


Map::Map() {
    kdTree = new KDTree();
}

Map::~Map() {
    // 释放所有点和道路的内存
    for (auto point : points) {
        delete point;
    }
    
    for (auto road : roads) {
        delete road;
    }
    
    delete kdTree;
}

void Map::addPoint(Point* point) {
    points.push_back(point);
    // 初始化该点的邻接表
    adjacencyList[point->getId()] = std::vector<Road*>();
}

void Map::addRoad(Road* road) {
    roads.push_back(road);
    
    // 更新邻接表
    int startId = road->getStartPoint()->getId();
    int endId = road->getEndPoint()->getId();
    
    adjacencyList[startId].push_back(road);
    // 假设道路是双向的
    adjacencyList[endId].push_back(road);
}

Point* Map::getPointById(int id) const {
    for (auto point : points) {
        if (point->getId() == id) {
            return point;
        }
    }
    return nullptr;
}

Road* Map::getRoadById(int id) const {
    for (auto road : roads) {
        if (road->getId() == id) {
            return road;
        }
    }
    return nullptr;
}

std::vector<Point*> Map::getAllPoints() const {
    return points;
}

std::vector<Road*> Map::getAllRoads() const {
    return roads;
}

std::vector<Road*> Map::getRoadsFromPoint(int pointId) const {
    auto it = adjacencyList.find(pointId);
    if (it != adjacencyList.end()) {
        return it->second;
    }
    return std::vector<Road*>();
}

Road* Map::getRoadBetweenPoints(int startId, int endId) const {
    auto roadsFromStart = getRoadsFromPoint(startId);
    
    for (auto road : roadsFromStart) {
        Point* start = road->getStartPoint();
        Point* end = road->getEndPoint();
        
        if ((start->getId() == startId && end->getId() == endId) ||
            (start->getId() == endId && end->getId() == startId)) {
            return road;
        }
    }
    
    return nullptr;
}

std::vector<Point*> Map::getNearestPoints(double x, double y, int count) const {
    // 使用KD树查找最近的点
    return kdTree->findKNearest(x, y, count);
}

std::vector<Point*> Map::getAdjacentPoints(int pointId) const {
    std::vector<Point*> adjacent;
    auto roads = getRoadsFromPoint(pointId);
    
    for (auto road : roads) {
        Point* start = road->getStartPoint();
        Point* end = road->getEndPoint();
        
        if (start->getId() == pointId) {
            adjacent.push_back(end);
        } else {
            adjacent.push_back(start);
        }
    }
    
    return adjacent;
}

bool Map::isConnected() const {
    if (points.empty()) {
        return true;  // 空图被认为是连通的
    }
    
    // 使用BFS检查图的连通性
    std::unordered_set<int> visited;
    std::queue<int> queue;
    
    // 从第一个点开始BFS
    int startId = points[0]->getId();
    queue.push(startId);
    visited.insert(startId);
    
    while (!queue.empty()) {
        int currentId = queue.front();
        queue.pop();
        
        auto roads = getRoadsFromPoint(currentId);
        for (auto road : roads) {
            Point* start = road->getStartPoint();
            Point* end = road->getEndPoint();
            
            int nextId = (start->getId() == currentId) ? end->getId() : start->getId();
            
            if (visited.find(nextId) == visited.end()) {
                visited.insert(nextId);
                queue.push(nextId);
            }
        }
    }
    
    // 如果所有点都被访问到，则图是连通的
    return visited.size() == points.size();
}

void Map::rebuildKDTree() {
    // 使用批量构建方式，而不是逐个插入
    delete kdTree;
    kdTree = new KDTree();
    kdTree->build(points);
}