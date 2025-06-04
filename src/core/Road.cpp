#include "Road.h"
#include <cmath>

Road::Road(int id, Point* start, Point* end) : 
    id(id), 
    startPoint(start), 
    endPoint(end), 
    capacity(5),  // 默认容量值
    currentCars(0)  // 初始车辆数为0
{
    // 计算道路长度为两点之间的距离
    length = start->distanceTo(*end);
}

int Road::getId() const {
    return id;
}

Point* Road::getStartPoint() const {
    return startPoint;
}

Point* Road::getEndPoint() const {
    return endPoint;
}

double Road::getLength() const {
    return length;
}

int Road::getCapacity() const {
    return capacity;
}

int Road::getCurrentCars() const {
    return currentCars;
}

void Road::setCapacity(int v) {
    capacity = v;
}

void Road::setCurrentCars(int n) {
    currentCars = n;
}

double Road::getTravelTime(double c, double threshold) const {
    double ratio = static_cast<double>(currentCars) / capacity;
    double factor = 1.0;
    
    // 当车流量/容量比超过阈值时，拥堵因子增加
    if (ratio > threshold) {
        factor = 1.0 + exp(ratio - threshold);
    }
    
    // 计算通行时间：c * 长度 * 拥堵因子
    return c * length * factor;
}