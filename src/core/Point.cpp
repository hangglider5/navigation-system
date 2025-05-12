#include "Point.h"
#include <cmath>

Point::Point(int id, double x, double y) : id(id), x(x), y(y) {}

int Point::getId() const {
    return id;
}

double Point::getX() const {
    return x;
}

double Point::getY() const {
    return y;
}

double Point::distanceTo(const Point& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}