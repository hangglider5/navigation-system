#include "PathFinder.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <algorithm>

PathFinder::PathFinder(Map* map) : map(map) {
}

std::vector<Point*> PathFinder::findShortestPath(int startPointId, int endPointId) const {
    // 使用Dijkstra算法找最短路径
    std::unordered_map<int, double> distance;
    std::unordered_map<int, int> previous;
    std::unordered_set<int> visited;
    
    // 优先队列，按距离排序
    auto compare = [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
    };
    std::priority_queue<std::pair<int, double>, 
                        std::vector<std::pair<int, double>>, 
                        decltype(compare)> queue(compare);
    
    // 初始化距离
    for (auto point : map->getAllPoints()) {
        int id = point->getId();
        distance[id] = std::numeric_limits<double>::infinity();
        previous[id] = -1;
    }
    
    distance[startPointId] = 0;
    queue.push(std::make_pair(startPointId, 0));
    
    while (!queue.empty()) {
        int currentId = queue.top().first;
        queue.pop();
        
        // 如果已经到达终点，结束搜索
        if (currentId == endPointId) {
            break;
        }
        
        // 如果已经访问过该点，跳过
        if (visited.find(currentId) != visited.end()) {
            continue;
        }
        
        visited.insert(currentId);
        
        // 遍历所有相邻点
        for (auto adjPoint : map->getAdjacentPoints(currentId)) {
            int adjId = adjPoint->getId();
            
            // 如果已经访问过该点，跳过
            if (visited.find(adjId) != visited.end()) {
                continue;
            }
            
            // 获取连接两点的道路
            Road* road = map->getRoadBetweenPoints(currentId, adjId);
            if (!road) {
                continue;
            }
            
            // 计算新的距离
            double newDistance = distance[currentId] + road->getLength();
            
            // 如果找到更短的路径，更新距离和前驱
            if (newDistance < distance[adjId]) {
                distance[adjId] = newDistance;
                previous[adjId] = currentId;
                queue.push(std::make_pair(adjId, newDistance));
            }
        }
    }
    
    // 重建路径
    std::vector<Point*> path;
    for (int at = endPointId; at != -1; at = previous[at]) {
        path.push_back(map->getPointById(at));
    }
    
    // 反转路径，使其从起点到终点
    std::reverse(path.begin(), path.end());
    
    return path;
}

std::vector<Point*> PathFinder::findFastestPath(int startPointId, int endPointId, double c, double threshold) const {
    // 使用Dijkstra算法找最快路径，考虑路况
    std::unordered_map<int, double> travelTime;
    std::unordered_map<int, int> previous;
    std::unordered_set<int> visited;
    
    // 优先队列，按行驶时间排序
    auto compare = [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
    };
    std::priority_queue<std::pair<int, double>, 
                        std::vector<std::pair<int, double>>, 
                        decltype(compare)> queue(compare);
    
    // 初始化行驶时间
    for (auto point : map->getAllPoints()) {
        int id = point->getId();
        travelTime[id] = std::numeric_limits<double>::infinity();
        previous[id] = -1;
    }
    
    travelTime[startPointId] = 0;
    queue.push(std::make_pair(startPointId, 0));
    
    while (!queue.empty()) {
        int currentId = queue.top().first;
        queue.pop();
        
        // 如果已经到达终点，结束搜索
        if (currentId == endPointId) {
            break;
        }
        
        // 如果已经访问过该点，跳过
        if (visited.find(currentId) != visited.end()) {
            continue;
        }
        
        visited.insert(currentId);
        
        // 遍历所有相邻点
        for (auto adjPoint : map->getAdjacentPoints(currentId)) {
            int adjId = adjPoint->getId();
            
            // 如果已经访问过该点，跳过
            if (visited.find(adjId) != visited.end()) {
                continue;
            }
            
            // 获取连接两点的道路
            Road* road = map->getRoadBetweenPoints(currentId, adjId);
            if (!road) {
                continue;
            }
            
            // 计算行驶时间，考虑路况
            double roadTravelTime = road->getTravelTime(c, threshold);
            double newTravelTime = travelTime[currentId] + roadTravelTime;
            
            // 如果找到更快的路径，更新时间和前驱
            if (newTravelTime < travelTime[adjId]) {
                travelTime[adjId] = newTravelTime;
                previous[adjId] = currentId;
                queue.push(std::make_pair(adjId, newTravelTime));
            }
        }
    }
    
    // 重建路径
    std::vector<Point*> path;
    for (int at = endPointId; at != -1; at = previous[at]) {
        path.push_back(map->getPointById(at));
    }
    
    // 反转路径，使其从起点到终点
    std::reverse(path.begin(), path.end());
    
    return path;
}

std::vector<Road*> PathFinder::getRoadsInPath(const std::vector<Point*>& path) const {
    std::vector<Road*> roads;
    
    // 如果路径少于2个点，无法形成道路
    if (path.size() < 2) {
        return roads;
    }
    
    // 遍历路径中的每对相邻点，找出连接它们的道路
    for (size_t i = 0; i < path.size() - 1; i++) {
        int startId = path[i]->getId();
        int endId = path[i + 1]->getId();
        
        Road* road = map->getRoadBetweenPoints(startId, endId);
        if (road) {
            roads.push_back(road);
        }
    }
    
    return roads;
}

double PathFinder::calculatePathLength(const std::vector<Point*>& path) const {
    double totalLength = 0.0;
    
    // 获取路径上的所有道路
    std::vector<Road*> roads = getRoadsInPath(path);
    
    // 累加所有道路的长度
    for (auto road : roads) {
        totalLength += road->getLength();
    }
    
    return totalLength;
}

double PathFinder::calculatePathTravelTime(const std::vector<Point*>& path, double c, double threshold) const {
    double totalTime = 0.0;
    
    // 获取路径上的所有道路
    std::vector<Road*> roads = getRoadsInPath(path);
    
    // 累加所有道路的行驶时间
    for (auto road : roads) {
        totalTime += road->getTravelTime(c, threshold);
    }
    
    return totalTime;
}