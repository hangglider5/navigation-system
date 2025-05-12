#include "KDTree.h"
#include <queue>
#include <numeric>
#include <algorithm> // 添加这个头文件以使用std::nth_element

KDTree::KDTree() : root(nullptr), dimensions(2) {}

KDTree::~KDTree() {
    deleteTree(root);
}

void KDTree::deleteTree(KDNode* node) {
    if (node) {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}

double KDTree::calculateVariance(const std::vector<Point*>& points, int start, int end, int axis) {
    if (start > end) return 0.0;
    
    double sum = 0.0;
    double sumSq = 0.0;
    int count = end - start + 1;
    
    for (int i = start; i <= end; i++) {
        double value = (axis == 0) ? points[i]->getX() : points[i]->getY();
        sum += value;
        sumSq += value * value;
    }
    
    double mean = sum / count;
    return (sumSq / count) - (mean * mean);
}

int KDTree::selectBestSplitAxis(const std::vector<Point*>& points, int start, int end) {
    double xVariance = calculateVariance(points, start, end, 0);
    double yVariance = calculateVariance(points, start, end, 1);
    
    return (xVariance >= yVariance) ? 0 : 1;
}

// 使用std::nth_element替代quickSelect
int KDTree::quickSelect(std::vector<Point*>& points, int start, int end, int k, int axis) {
    // 使用lambda表达式定义比较函数，根据指定轴进行比较
    auto compareByAxis = [axis](const Point* a, const Point* b) {
        double aValue = (axis == 0) ? a->getX() : a->getY();
        double bValue = (axis == 0) ? b->getX() : b->getY();
        return aValue < bValue;
    };
    
    // 使用std::nth_element找到第k个元素
    std::nth_element(points.begin() + start, points.begin() + k, points.begin() + end + 1, compareByAxis);
    
    return k;
}

void KDTree::build(const std::vector<Point*>& points) {
    if (points.empty()) {
        return;
    }
    
    // 复制点集合，因为我们需要对其进行排序
    std::vector<Point*> pointsCopy = points;
    
    // 重新排列点以提高缓存效率
    reorderPointsForCacheEfficiency(pointsCopy);
    
    // 使用串行版本构建树
    root = buildTree(pointsCopy, 0, 0, pointsCopy.size() - 1);
}

KDNode* KDTree::buildTree(std::vector<Point*>& points, int depth, int start, int end) {
    if (start > end) {
        return nullptr;
    }
    
    // 选择最佳分割维度
    int axis = selectBestSplitAxis(points, start, end);
    
    // 选择中位数作为分割点
    int mid = start + (end - start) / 2;
    
    // 使用快速选择算法找到中位数
    int pivotIndex = quickSelect(points, start, end, mid, axis);
    
    // 创建节点，并保存分割维度
    KDNode* node = new KDNode(points[pivotIndex], axis);
    
    // 递归构建左右子树
    node->left = buildTree(points, depth + 1, start, pivotIndex - 1);
    node->right = buildTree(points, depth + 1, pivotIndex + 1, end);
    
    return node;
}

Point* KDTree::findNearest(double x, double y) {
    if (!root) {
        return nullptr;
    }
    
    Point target(-1, x, y);
    Point* nearest = nullptr;
    double bestDist = std::numeric_limits<double>::max();
    
    nearestNeighborSearch(root, target, 0, nearest, bestDist);
    
    return nearest;
}

void KDTree::nearestNeighborSearch(KDNode* node, const Point& target, int depth, 
                                  Point*& nearest, double& bestDist) {
    if (!node) {
        return;
    }
    
    // 使用节点中记录的分割维度，而不是depth % dimensions
    int axis = node->splitAxis;
    
    // 计算当前点到目标的距离
    double dist = target.distanceTo(*(node->point));
    
    // 如果当前点更近，更新最近点
    if (dist < bestDist) {
        bestDist = dist;
        nearest = node->point;
    }
    
    // 确定搜索方向
    double axisValue = (axis == 0) ? target.getX() : target.getY();
    double nodeValue = (axis == 0) ? node->point->getX() : node->point->getY();
    
    KDNode* nearBranch = (axisValue < nodeValue) ? node->left : node->right;
    KDNode* farBranch = (axisValue < nodeValue) ? node->right : node->left;
    
    // 先搜索更可能包含最近点的分支
    nearestNeighborSearch(nearBranch, target, depth + 1, nearest, bestDist);
    
    // 计算目标点到分割超平面的距离
    double planeDistance = std::abs(axisValue - nodeValue);
    
    // 检查是否需要搜索另一个分支
    if (planeDistance * planeDistance < bestDist) {
        nearestNeighborSearch(farBranch, target, depth + 1, nearest, bestDist);
    }
}

std::vector<Point*> KDTree::findKNearest(double x, double y, int k) {
    if (!root || k <= 0) {
        return std::vector<Point*>();
    }
    
    Point target(-1, x, y);
    
    // 修改：使用默认的最大优先队列 (std::less by default)
    std::priority_queue<std::pair<double, Point*>> nearestPoints; 
    
    kNearestNeighborSearch(root, target, 0, nearestPoints, k);
    
    // 提取点
    std::vector<Point*> result;
    result.reserve(nearestPoints.size()); // 预分配空间，大小为实际找到的点数
    
    while (!nearestPoints.empty()) {
        result.push_back(nearestPoints.top().second);
        nearestPoints.pop();
    }
    // 结果中的点顺序是按距离从大到小排列的。如果需要从小到大，可以反转。
    std::reverse(result.begin(), result.end()); 
    
    return result;
}

void KDTree::kNearestNeighborSearch(KDNode* node, const Point& target, int depth, 
                                   std::priority_queue<std::pair<double, Point*>>& nearestPoints, // 修改：匹配最大优先队列
                                   int k) {
    if (!node) {
        return;
    }
    
    // 修改：使用节点存储的分割轴
    int axis = node->splitAxis; 
    
    // 计算当前点到目标的距离
    double dist = target.distanceTo(*(node->point));
    
    // 修改：更新最大优先队列的逻辑
    if (nearestPoints.size() < k) {
        nearestPoints.push(std::make_pair(dist, node->point));
    } else if (dist < nearestPoints.top().first) { // 如果当前点比队列中距离最大的点还要近
        nearestPoints.pop(); // 移除当前K个点中距离最大的
        nearestPoints.push(std::make_pair(dist, node->point)); // 插入当前点
    }
    
    // 确定搜索方向
    double axisValue = (axis == 0) ? target.getX() : target.getY();
    double nodeValue = (axis == 0) ? node->point->getX() : node->point->getY();
    
    KDNode* nearBranch = (axisValue < nodeValue) ? node->left : node->right;
    KDNode* farBranch = (axisValue < nodeValue) ? node->right : node->left;
    
    // 先搜索更可能包含最近点的分支
    kNearestNeighborSearch(nearBranch, target, depth + 1, nearestPoints, k);
    
    // 修改：剪枝逻辑，配合最大优先队列
    // current_kth_distance 是当前找到的K个点中，距离目标点最远的那个点的距离
    double current_kth_distance = std::numeric_limits<double>::max();
    if (nearestPoints.size() == k) {
        current_kth_distance = nearestPoints.top().first;
    }
    
    // 计算目标点到分割超平面的距离
    double planeDistance = std::abs(axisValue - nodeValue);
    
    // 检查是否需要搜索另一个分支
    // 只有当 “目标点到分割超平面的距离” 小于 “当前已找到的第K近的点的距离”
    // 或者 当我们还没找够K个点时，才需要搜索另一分支
    if (planeDistance < current_kth_distance || nearestPoints.size() < k) {
        kNearestNeighborSearch(farBranch, target, depth + 1, nearestPoints, k);
    }
}

bool KDTree::sphereBoundsOverlap(const Point& center, double radius, double minBounds[2], double maxBounds[2]) {
    double distSq = 0;
    
    for (int i = 0; i < dimensions; i++) {
        double coord = (i == 0) ? center.getX() : center.getY();
        double minCoord = minBounds[i];
        double maxCoord = maxBounds[i];
        
        if (coord < minCoord) {
            distSq += (minCoord - coord) * (minCoord - coord);
        } else if (coord > maxCoord) {
            distSq += (coord - maxCoord) * (coord - maxCoord);
        }
    }
    
    return distSq <= radius * radius;
}

void KDTree::reorderPointsForCacheEfficiency(std::vector<Point*>& points) {
    if (points.size() <= 1) return;
    
    std::vector<Point*> reordered;
    reordered.reserve(points.size());
    
    // 使用层序遍历重新排列点
    std::queue<std::pair<int, int>> queue;
    queue.push({0, static_cast<int>(points.size() - 1)});
    
    while (!queue.empty()) {
        auto [start, end] = queue.front();
        queue.pop();
        
        if (start <= end) {
            int mid = start + (end - start) / 2;
            reordered.push_back(points[mid]);
            
            queue.push({start, mid - 1});
            queue.push({mid + 1, end});
        }
    }
    
    points = reordered;
}