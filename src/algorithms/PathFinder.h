#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include <vector>
#include "../core/Map.h"
#include "../core/Point.h"
#include "../core/Road.h"

class PathFinder {
private:
    Map* map;
    
public:
    PathFinder(Map* map);
    
    // 计算两点之间的最短路径（基于距离）
    std::vector<Point*> findShortestPath(int startPointId, int endPointId) const;
    
    // 计算两点之间的最快路径（考虑路况）
    std::vector<Point*> findFastestPath(int startPointId, int endPointId, double c, double threshold) const;
    
    // 获取路径上的所有道路
    std::vector<Road*> getRoadsInPath(const std::vector<Point*>& path) const;
    
    // 计算路径总长度
    double calculatePathLength(const std::vector<Point*>& path) const;
    
    // 计算路径总行驶时间（考虑路况）
    double calculatePathTravelTime(const std::vector<Point*>& path, double c, double threshold) const;
};

#endif // PATH_FINDER_H