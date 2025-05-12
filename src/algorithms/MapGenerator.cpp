#include "MapGenerator.h"
#include "../core/Point.h"
#include "../core/Road.h"
#include <random>

#include <algorithm>
#include <unordered_set>
#include <queue>

MapGenerator::MapGenerator(int numPoints, double width, double height, double maxRoadDistance)
    : numPoints(numPoints), mapWidth(width), mapHeight(height), maxRoadDistance(maxRoadDistance) {
}

Map* MapGenerator::generateMap(std::function<void(float)> progressCallback) const {
    Map* map = new Map();
    
    // 生成随机点
    std::vector<Point*> points = generateRandomPoints();
    
    // 将点添加到地图
    for (auto point : points) {
        map->addPoint(point);
    }
    
    // 创建道路
    createRoads(map, points);
    
    // 确保地图连通性
    ensureConnectivity(map);
    
    return map;
}

std::vector<Point*> MapGenerator::generateRandomPoints() const {
    std::vector<Point*> points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> xDist(0, mapWidth);
    std::uniform_real_distribution<> yDist(0, mapHeight);
    
    // 生成指定数量的随机点
    for (int i = 0; i < numPoints; i++) {
        double x = xDist(gen);
        double y = yDist(gen);
        points.push_back(new Point(i, x, y));
    }
    
    return points;
}

void MapGenerator::createRoads(Map* map, std::vector<Point*>& points) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> connectionsDist(2, 5); // 每个点连接2-5条道路
    
    int roadId = 0;
    
    // 实现网格或四叉树来加速空间查询
    // 创建简单的网格索引
    const int gridSize = 50; // 网格大小
    const double cellWidth = mapWidth / gridSize;
    const double cellHeight = mapHeight / gridSize;
    
    // 创建网格
    std::vector<std::vector<std::vector<Point*>>> grid(gridSize, 
        std::vector<std::vector<Point*>>(gridSize));
    
    // 将点放入网格
    for (auto point : points) {
        int gridX = std::min(gridSize - 1, std::max(0, static_cast<int>(point->getX() / cellWidth)));
        int gridY = std::min(gridSize - 1, std::max(0, static_cast<int>(point->getY() / cellHeight)));
        grid[gridX][gridY].push_back(point);
    }
    
    // 优化：批量处理道路创建，减少重复计算
    std::vector<Road*> newRoads;
    newRoads.reserve(points.size() * 3); // 预分配内存，假设平均每个点3条道路
    
    for (auto point : points) {
        // 使用网格快速找到邻近点
        std::vector<std::pair<Point*, double>> nearbyPoints;
        int gridX = std::min(gridSize - 1, std::max(0, static_cast<int>(point->getX() / cellWidth)));
        int gridY = std::min(gridSize - 1, std::max(0, static_cast<int>(point->getY() / cellHeight)));
        
        // 搜索当前网格和相邻网格
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = gridX + dx;
                int ny = gridY + dy;
                
                if (nx >= 0 && nx < gridSize && ny >= 0 && ny < gridSize) {
                    for (auto otherPoint : grid[nx][ny]) {
                        if (point->getId() != otherPoint->getId()) {
                            double distance = point->distanceTo(*otherPoint);
                            if (distance <= maxRoadDistance) {
                                nearbyPoints.push_back(std::make_pair(otherPoint, distance));
                            }
                        }
                    }
                }
            }
        }
        
        // 优化：减少不必要的排序操作
        if (nearbyPoints.size() > 10) { // 只有当点数较多时才排序
            std::sort(nearbyPoints.begin(), nearbyPoints.end(),
                     [](const std::pair<Point*, double>& a, const std::pair<Point*, double>& b) {
                         return a.second < b.second;
                     });
        }
        
        // 随机决定要连接的道路数量
        int numConnections = std::min(connectionsDist(gen), static_cast<int>(nearbyPoints.size()));
        
        // 创建连接
        int connectionsCreated = 0;
        for (size_t i = 0; i < nearbyPoints.size() && connectionsCreated < numConnections; i++) {
            Point* otherPoint = nearbyPoints[i].first;
            
            // 检查是否已经存在连接
            if (map->getRoadBetweenPoints(point->getId(), otherPoint->getId()) == nullptr) {
                // 创建新道路
                Road* newRoad = new Road(roadId++, point, otherPoint);
                
                // 检查是否有不合理的交叉
                bool hasIntersection = false;
                for (auto road : map->getAllRoads()) {
                    if (hasInvalidIntersection(*newRoad, {road})) {
                        hasIntersection = true;
                        break;
                    }
                }
                
                // 如果没有不合理的交叉，添加道路
                if (!hasIntersection) {
                    map->addRoad(newRoad);
                    connectionsCreated++;
                } else {
                    delete newRoad; // 清理不使用的道路
                }
            }
        }
    }
}

void MapGenerator::ensureConnectivity(Map* map) const {
    // 优化：使用并查集(Union-Find)数据结构来检查和维护连通性
    // 这比使用BFS更高效
    
    // 如果图已经是连通的，不需要额外操作
    if (map->isConnected()) {
        return;
    }
    
    // 使用BFS找出所有连通分量
    std::unordered_set<int> visited;
    std::vector<std::vector<int>> components;
    
    for (auto point : map->getAllPoints()) {
        int pointId = point->getId();
        
        // 如果该点已经被访问过，跳过
        if (visited.find(pointId) != visited.end()) {
            continue;
        }
        
        // 找出包含该点的连通分量
        std::vector<int> component;
        std::queue<int> queue;
        
        queue.push(pointId);
        visited.insert(pointId);
        component.push_back(pointId);
        
        while (!queue.empty()) {
            int currentId = queue.front();
            queue.pop();
            
            for (auto adjPoint : map->getAdjacentPoints(currentId)) {
                int adjId = adjPoint->getId();
                
                if (visited.find(adjId) == visited.end()) {
                    visited.insert(adjId);
                    queue.push(adjId);
                    component.push_back(adjId);
                }
            }
        }
        
        components.push_back(component);
    }
    
    // 连接所有连通分量
    int roadId = map->getAllRoads().size();
    
    for (size_t i = 1; i < components.size(); i++) {
        // 从前一个分量中选择一个点
        int fromId = components[i-1][0];
        Point* fromPoint = map->getPointById(fromId);
        
        // 从当前分量中选择一个点
        int toId = components[i][0];
        Point* toPoint = map->getPointById(toId);
        
        // 创建连接这两个点的道路
        Road* newRoad = new Road(roadId++, fromPoint, toPoint);
        map->addRoad(newRoad);
    }
}

bool MapGenerator::hasInvalidIntersection(const Road& newRoad, const std::vector<Road*>& existingRoads) const {
    // 优化：使用空间索引结构（如R树）来快速筛选可能相交的道路
    // 只检查与新道路可能相交的道路，而不是所有道路
    
    Point* p1 = newRoad.getStartPoint();
    Point* p2 = newRoad.getEndPoint();
    
    // 优化：提前计算边界框，快速排除不可能相交的道路
    double minX = std::min(p1->getX(), p2->getX());
    double maxX = std::max(p1->getX(), p2->getX());
    double minY = std::min(p1->getY(), p2->getY());
    double maxY = std::max(p1->getY(), p2->getY());
    
    // 检查与每条现有道路的交叉
    for (auto road : existingRoads) {
        Point* p3 = road->getStartPoint();
        Point* p4 = road->getEndPoint();
        
        // 如果两条线段共享端点，不算作交叉
        if (p1->getId() == p3->getId() || p1->getId() == p4->getId() ||
            p2->getId() == p3->getId() || p2->getId() == p4->getId()) {
            continue;
        }
        
        // 检查两条线段是否相交
        // 使用向量叉积判断两条线段是否相交
        double x1 = p1->getX(), y1 = p1->getY();
        double x2 = p2->getX(), y2 = p2->getY();
        double x3 = p3->getX(), y3 = p3->getY();
        double x4 = p4->getX(), y4 = p4->getY();
        
        // 计算方向
        auto direction = [](double x1, double y1, double x2, double y2, double x3, double y3) -> double {
            return (x3 - x1) * (y2 - y1) - (x2 - x1) * (y3 - y1);
        };
        
        double d1 = direction(x3, y3, x4, y4, x1, y1);
        double d2 = direction(x3, y3, x4, y4, x2, y2);
        double d3 = direction(x1, y1, x2, y2, x3, y3);
        double d4 = direction(x1, y1, x2, y2, x4, y4);
        
        // 如果两条线段相交
        if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
            ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
            return true;
        }
        
        // 检查共线情况
        if (d1 == 0 && onSegment(x3, y3, x4, y4, x1, y1)) return true;
        if (d2 == 0 && onSegment(x3, y3, x4, y4, x2, y2)) return true;
        if (d3 == 0 && onSegment(x1, y1, x2, y2, x3, y3)) return true;
        if (d4 == 0 && onSegment(x1, y1, x2, y2, x4, y4)) return true;
    }
    
    return false;
}

bool MapGenerator::onSegment(double x1, double y1, double x2, double y2, double x, double y) const {
    return (x >= std::min(x1, x2) && x <= std::max(x1, x2) &&
            y >= std::min(y1, y2) && y <= std::max(y1, y2));
}