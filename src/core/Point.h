#ifndef POINT_H
#define POINT_H

class Point {
private:
    int id;
    double x;
    double y;
    
public:
    Point(int id, double x, double y);
    
    int getId() const;
    double getX() const;
    double getY() const;
    
    // 计算到另一个点的距离
    double distanceTo(const Point& other) const;
};

#endif // POINT_H