#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <vector>
#include "core/Point.h"
#include "core/Road.h"
#include "algorithms/TrafficSimulator.h" // 添加这一行

class MapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    void setMapData(const std::vector<Point*>& points, const std::vector<Road*>& roads);
    void setSpecialPoint(const QPointF& coordinates); // 新增：设置特殊点
    void clearSpecialPoint();                       // 新增：清除特殊点
    void resetView(); // 新增：重置视图方法
    void setShortestPathData(const std::vector<Point*>& pathPoints, const std::vector<Road*>& pathRoads); // 新增：设置最短路径数据
    void clearShortestPath(); // 新增：清除最短路径数据
    void setZoomFactor(double factor); // <--- 新增：设置缩放因子
    
    // 新增：设置交通模拟器
    void setTrafficSimulator(TrafficSimulator* simulator);
    // 新增：更新交通流量显示
    void updateTrafficDisplay();
    // 新增：设置拥堵阈值
    void setCongestionThreshold(double threshold);
    // 新增：设置路径的起点和终点
    void setPathEndpoints(const QPointF& start, const QPointF& end);
    // 新增：清除路径端点
    void clearPathEndpoints();

protected:
    void paintEvent(QPaintEvent *event) override;
    // 新增：鼠标事件处理函数声明
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    std::vector<Point*> displayPoints;
    std::vector<Road*> displayRoads;
    // 你可能需要添加缩放和平移的成员变量和逻辑
    double scaleFactor = 1.0;
    QPointF panOffset = QPointF(0, 0);
    bool hasSpecialMarkedPoint = false;
    QPointF specialPoint;
    
    // 最短路径数据
    std::vector<Point*> pathPoints;
    std::vector<Road*> pathRoads;
    bool hasShortestPath = false;
    
    // 新增：路径起点和终点
    bool hasPathEndpoints = false;
    QPointF pathStartPoint;
    QPointF pathEndPoint;
    
    // 新增：交通模拟相关
    TrafficSimulator* trafficSimulator = nullptr;
    double congestionThreshold = 0.7; // 默认拥堵阈值
    
    // 新增：根据拥堵程度获取颜色
    QColor getTrafficColor(double congestionLevel) const;

    // 新增：用于平移的最后鼠标位置
    QPoint lastMousePos;
};

#endif // MAPWIDGET_H
