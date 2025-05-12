#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include "../core/Map.h"
#include <functional> // 添加这个头文件以支持 std::function

class MapGenerator {
private:
    int numPoints;
    double mapWidth;
    double mapHeight;
    double maxRoadDistance;
    
public:
    MapGenerator(int numPoints, double width, double height, double maxRoadDistance);
    
    // 修改声明，添加 progressCallback 参数
    Map* generateMap(std::function<void(float)> progressCallback = nullptr) const;
    
private:
    // 随机生成点
    std::vector<Point*> generateRandomPoints() const;
    
    // 为每个点创建连接到附近点的道路
    void createRoads(Map* map, std::vector<Point*>& points) const;
    
    // 确保图是连通的
    void ensureConnectivity(Map* map) const;
    
    // 检查道路是否有不合理的交叉
    bool hasInvalidIntersection(const Road& newRoad, const std::vector<Road*>& existingRoads) const;
    
    // 判断点是否在线段上
    bool onSegment(double x1, double y1, double x2, double y2, double x, double y) const;
};

#endif // MAP_GENERATOR_H