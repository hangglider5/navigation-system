#ifndef MAP_RENDERER_H
#define MAP_RENDERER_H

#include <vector>
#include "../core/Map.h"
#include "../algorithms/TrafficSimulator.h"

class MapRenderer {
private:
    Map* map;
    TrafficSimulator* trafficSimulator;
    int viewportWidth;
    int viewportHeight;
    double centerX;
    double centerY;
    double zoomLevel;
    
public:
    MapRenderer(Map* map, TrafficSimulator* trafficSimulator, int viewportWidth, int viewportHeight);
    
    void setMap(Map* map);
    void setTrafficSimulator(TrafficSimulator* trafficSimulator);
    
    // 渲染整个地图
    void renderMap();
    
    // 渲染指定位置附近的地图
    void renderNearLocation(double x, double y);
    
    // 渲染指定位置附近的地图，显示最近的点和边
    void renderNearLocation(double x, double y, const std::vector<Point*>& nearPoints);
    
    // 高亮显示路径
    void highlightPath(const std::vector<Point*>& path);
    
    // 渲染带有交通流量的道路
    void renderRoadsWithTraffic();
    
    // 缩放地图
    void zoom(double factor);
    
    // 平移地图
    void pan(double deltaX, double deltaY);
};

#endif // MAP_RENDERER_H