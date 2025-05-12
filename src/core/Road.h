#ifndef ROAD_H
#define ROAD_H

#include "Point.h"

class Road {
private:
    int id;
    Point* startPoint;
    Point* endPoint;
    double length;      // 道路长度
    int capacity;       // 车容量v
    int currentCars;    // 当前车辆数n
    
public:
    Road(int id, Point* start, Point* end);
    
    int getId() const;
    Point* getStartPoint() const;
    Point* getEndPoint() const;
    double getLength() const;
    int getCapacity() const;
    int getCurrentCars() const;
    
    void setCapacity(int v);
    void setCurrentCars(int n);
    
    // 计算通行时间
    double getTravelTime(double c, double threshold) const;
};

#endif // ROAD_H