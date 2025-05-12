#include "MapRenderer.h"
#include <iostream>
#include <unordered_set>  // 添加这一行

MapRenderer::MapRenderer(Map* map, TrafficSimulator* trafficSimulator, int viewportWidth, int viewportHeight)
    : map(map), trafficSimulator(trafficSimulator), 
      viewportWidth(viewportWidth), viewportHeight(viewportHeight),
      centerX(500.0), centerY(500.0), zoomLevel(1.0) {
}

void MapRenderer::setMap(Map* map) {
    this->map = map;
}

void MapRenderer::setTrafficSimulator(TrafficSimulator* trafficSimulator) {
    this->trafficSimulator = trafficSimulator;
}

void MapRenderer::renderMap() {
    if (!map) {
        std::cout << "错误：地图未设置！" << std::endl;
        return;
    }
    
    std::cout << "渲染地图..." << std::endl;
    // 实现地图渲染逻辑
    // ...
}

void MapRenderer::renderNearLocation(double x, double y, const std::vector<Point*>& nearPoints) {
    if (!map) {
        std::cout << "错误：地图未设置！" << std::endl;
        return;
    }
    
    // 更新中心点
    centerX = x;
    centerY = y;
    
    std::cout << "渲染位置 (" << x << ", " << y << ") 附近的地图..." << std::endl;
    
    if (nearPoints.empty()) {
        std::cout << "附近没有点！" << std::endl;
        return;
    }
    
    // 显示最近的点
    std::cout << "附近的点：" << std::endl;
    for (auto point : nearPoints) {
        std::cout << "点 " << point->getId() << ": (" 
                  << point->getX() << ", " << point->getY() << ")" << std::endl;
    }
    
    // 显示这些点之间的道路
    std::cout << "相关的道路：" << std::endl;
    std::unordered_set<int> roadIds; // 用于避免重复显示道路
    
    for (auto point : nearPoints) {
        auto roads = map->getRoadsFromPoint(point->getId());
        for (auto road : roads) {
            // 检查道路的另一端是否也在最近的点中
            Point* otherEnd = (road->getStartPoint()->getId() == point->getId()) ? 
                              road->getEndPoint() : road->getStartPoint();
            
            // 检查是否已经显示过这条道路
            if (roadIds.find(road->getId()) != roadIds.end()) {
                continue;
            }
            
            // 检查另一端是否在最近的点中
            bool otherEndInNearPoints = false;
            for (auto nearPoint : nearPoints) {
                if (nearPoint->getId() == otherEnd->getId()) {
                    otherEndInNearPoints = true;
                    break;
                }
            }
            
            if (otherEndInNearPoints) {
                std::cout << "道路 " << road->getId() << ": 从点 " 
                          << road->getStartPoint()->getId() << " 到点 " 
                          << road->getEndPoint()->getId() << std::endl;
                roadIds.insert(road->getId());
            }
        }
    }
}

void MapRenderer::highlightPath(const std::vector<Point*>& path) {
    if (!map) {
        std::cout << "错误：地图未设置！" << std::endl;
        return;
    }
    
    if (path.empty()) {
        std::cout << "路径为空，无法高亮显示！" << std::endl;
        return;
    }
    
    std::cout << "高亮显示路径..." << std::endl;
    // 实现路径高亮逻辑
    // ...
}

void MapRenderer::renderRoadsWithTraffic() {
    if (!map || !trafficSimulator) {
        std::cout << "错误：地图或交通模拟器未设置！" << std::endl;
        return;
    }
    
    std::cout << "渲染带有交通流量的道路..." << std::endl;
    // 实现带有交通流量的道路渲染逻辑
    // ...
}

void MapRenderer::zoom(double factor) {
    zoomLevel *= factor;
    std::cout << "地图缩放级别: " << zoomLevel << std::endl;
    // 实现缩放逻辑
    // ...
}

void MapRenderer::pan(double deltaX, double deltaY) {
    centerX += deltaX;
    centerY += deltaY;
    std::cout << "地图中心点: (" << centerX << ", " << centerY << ")" << std::endl;
    // 实现平移逻辑
    // ...
}