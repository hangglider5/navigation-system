#include "MapWidget.h"
#include <QPainter>
#include <QMouseEvent> // For QMouseEvent and QWheelEvent
#include <QWheelEvent> // For QWheelEvent

MapWidget::MapWidget(QWidget *parent) : QWidget(parent), hasSpecialMarkedPoint(false) { // 初始化新增的成员变量
    // 背景设为白色，方便观察
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
}

void MapWidget::setMapData(const std::vector<Point*>& points, const std::vector<Road*>& roads) {
    displayPoints = points;
    displayRoads = roads;
    // 当地图数据更新时，可以选择是否清除特殊标记点，
    // 或者保留它。这里我们暂时不清除，如果需要清除，可以调用 clearSpecialPoint()
    update(); // 请求重绘
}

// 新增：设置特殊点的方法
void MapWidget::setSpecialPoint(const QPointF& coordinates) {
    this->specialPoint = coordinates; // 将 QPointF 类型的 coordinates 赋值给 QPointF 类型的成员 specialPoint
    this->hasSpecialMarkedPoint = true; // 设置布尔标记为 true
    update(); // 请求重绘以显示特殊点
}

// 新增：清除特殊点的方法
void MapWidget::clearSpecialPoint() {
    hasSpecialMarkedPoint = false;
    update(); // 请求重绘以移除特殊点
}

// 新增：设置最短路径数据的方法
void MapWidget::setShortestPathData(const std::vector<Point*>& pathPoints, const std::vector<Road*>& pathRoads) {
    this->pathPoints = pathPoints; // 修改: shortestPathPoints -> this->pathPoints
    this->pathRoads = pathRoads;   // 修改: shortestPathRoads -> this->pathRoads
    hasShortestPath = !this->pathPoints.empty() && !this->pathRoads.empty(); // 确保使用成员变量
    update(); // 请求重绘以显示路径
}

// 新增：清除最短路径数据的方法
void MapWidget::clearShortestPath() {
    this->pathPoints.clear(); // 修改: shortestPathPoints -> this->pathPoints
    this->pathRoads.clear();  // 修改: shortestPathRoads -> this->pathRoads
    hasShortestPath = false;
    update(); // 请求重绘以移除路径
}

void MapWidget::resetView() {
    // 重置缩放和平移到默认值
    scaleFactor = 1.0;
    panOffset = QPointF(0,0);
    // 你可能还需要根据所有点的位置计算一个合适的初始缩放和中心点
    // 这里简单重置为默认
    update();
}

// 新增：设置缩放因子的方法
void MapWidget::setZoomFactor(double factor) {
    // 可以设置缩放的上下限
    if (factor < 0.1) factor = 0.1;
    if (factor > 10.0) factor = 10.0; // 限制最大缩放为10倍，可调整

    // 缩放时以当前视图中心为焦点，或者以鼠标位置为焦点（如果通过滚轮缩放）
    // 这里我们简单地改变 scaleFactor，平移焦点保持不变
    // 如果需要更复杂的缩放行为（例如，以鼠标指针为中心缩放），则需要更复杂的逻辑
    // QPointF viewCenter = mapToGlobal(rect().center()); // 视图中心
    // QPointF sceneCenter = mapToScene(viewCenter); // 场景中心点

    scaleFactor = factor;
    update(); // 请求重绘
}

void MapWidget::setTrafficSimulator(TrafficSimulator* simulator) {
    trafficSimulator = simulator;
    update(); // 更新显示
}

void MapWidget::updateTrafficDisplay() {
    update(); // 请求重绘
}

void MapWidget::setCongestionThreshold(double threshold) {
    congestionThreshold = threshold;
    update(); // 更新显示
}

QColor MapWidget::getTrafficColor(double congestionLevel) const {
    // 根据拥堵程度返回不同的颜色
    // 绿色 -> 黄色 -> 红色
    if (congestionLevel < 0.2) {
        // 畅通 - 绿色
        return QColor(0, 255, 0);
    } else if (congestionLevel < congestionThreshold) {
        // 中等 - 黄色
        int red = 255 * (congestionLevel - 0.3) / (congestionThreshold - 0.3);
        return QColor(red, 255, 0);
    } else {
        // 拥堵 - 红色
        int green = 255 * (1.0 - std::min(1.0, (congestionLevel - congestionThreshold) / 0.5));
        return QColor(255, green, 0);
    }
}

// 新增：设置路径的起点和终点
void MapWidget::setPathEndpoints(const QPointF& start, const QPointF& end) {
    pathStartPoint = start;
    pathEndPoint = end;
    hasPathEndpoints = true;
    update(); // 请求重绘以显示端点
}

// 新增：清除路径端点
void MapWidget::clearPathEndpoints() {
    hasPathEndpoints = false;
    update(); // 请求重绘以移除端点
}

void MapWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 应用缩放和平移
    painter.translate(panOffset);
    painter.scale(scaleFactor, scaleFactor);
    
    // 绘制道路
    QPen roadPen(Qt::black, 1);
    
    for (Road* road : displayRoads) {
        Point* start = road->getStartPoint();
        Point* end = road->getEndPoint();
        
        // 如果有交通模拟器，根据拥堵程度设置道路颜色
        if (trafficSimulator) {
            double congestionLevel = trafficSimulator->getCongestionLevel(road->getId());
            QColor roadColor = getTrafficColor(congestionLevel);
            
            // 设置线宽根据车流量变化
            int lineWidth = 1 + std::min(5, road->getCurrentCars() / 10);
            roadPen.setColor(roadColor);
            roadPen.setWidth(lineWidth);
        } else {
            roadPen.setColor(Qt::black);
            roadPen.setWidth(1);
        }
        
        painter.setPen(roadPen);
        painter.drawLine(start->getX(), start->getY(), end->getX(), end->getY());
        
        // 如果有交通模拟器，显示车流量信息
        if (trafficSimulator) {
            // 计算道路中点位置
            double midX = (start->getX() + end->getX()) / 2;
            double midY = (start->getY() + end->getY()) / 2;
        }
    }
    
    // 绘制所有点 (常规)
    painter.setPen(Qt::black); 
    painter.setBrush(Qt::black); 
    for (const auto& point : displayPoints) {
        if (point) {
            painter.drawEllipse(QPointF(point->getX(), point->getY()), 3, 3); 
        }
    }

    // 绘制最短路径
    if (hasShortestPath) {
        // 绘制路径上的道路 
        painter.setPen(QPen(Qt::green, 4)); // 路径道路颜色和宽度
        for (const auto& road : this->pathRoads) { // 修改: shortestPathRoads -> this->pathRoads
            if (road && road->getStartPoint() && road->getEndPoint()) {
                Point* p1 = road->getStartPoint();
                Point* p2 = road->getEndPoint();
                painter.drawLine(QPointF(p1->getX(), p1->getY()), QPointF(p2->getX(), p2->getY()));
            }
        }

        // 绘制路径上的点 
        painter.setPen(Qt::black);
        painter.setBrush(Qt::green); // 路径点填充颜色
        for (const auto& point : this->pathPoints) { // 修改: shortestPathPoints -> this->pathPoints
            if (point) {
                painter.drawEllipse(QPointF(point->getX(), point->getY()), 4, 4); // 路径点大小
            }
        }
    }

    // 绘制特殊标记点 (保持在最上层)
    if (hasSpecialMarkedPoint) {
        painter.setPen(QPen(Qt::black, 1)); 
        painter.setBrush(Qt::red);         
        painter.drawEllipse(this->specialPoint, 5, 5); // 修改: specialMarkedPoint -> this->specialPoint
    }
    
    // 绘制路径的起点和终点 (红色)
    if (hasPathEndpoints && hasShortestPath && !this->pathPoints.empty()) {
        painter.setPen(QPen(Qt::black, 1));
        painter.setBrush(Qt::red);
        
        // 绘制起点 (如果有)
        if (!this->pathPoints.empty()) {
            Point* startPoint = this->pathPoints.front();
            painter.drawEllipse(QPointF(startPoint->getX(), startPoint->getY()), 6, 6);
        }
        
        // 绘制终点 (如果有)
        if (this->pathPoints.size() > 1) {
            Point* endPoint = this->pathPoints.back();
            painter.drawEllipse(QPointF(endPoint->getX(), endPoint->getY()), 6, 6);
        }
    }
    
    // 如果有交通模拟器，绘制车辆位置
    if (trafficSimulator) {
        auto carPositions = trafficSimulator->getAllCarPositions();
        QPen carPen(Qt::blue, 3);
        painter.setPen(carPen);
        
        for (const auto& carPos : carPositions) {
            std::shared_ptr<Point> pos = carPos.second;
            painter.drawEllipse(QPointF(pos->getX(), pos->getY()), 3, 3);
        }
    }
}

void MapWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        lastMousePos = event->pos();
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - lastMousePos;
        panOffset += delta;
        lastMousePos = event->pos();
        update();
    }
}

void MapWidget::wheelEvent(QWheelEvent *event) {
    int degrees = event->angleDelta().y() / 8;
    int steps = degrees / 15; // 通常滚轮一步是15度

    double zoomFactor = 1.0;
    if (steps > 0) {
        zoomFactor = 1.1; // 放大
    } else if (steps < 0) {
        zoomFactor = 1.0 / 1.1; // 缩小
    }

    scaleFactor *= zoomFactor;
    update();
}
