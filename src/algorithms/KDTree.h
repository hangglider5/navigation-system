#ifndef KDTREE_H
#define KDTREE_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <memory>
// 移除 #include <omp.h>
#include "../core/Point.h"

// KD树节点
struct KDNode {
    Point* point;
    KDNode* left;
    KDNode* right;
    int splitAxis;  // 添加分割维度字段
    
    KDNode(Point* p, int axis = 0) : point(p), left(nullptr), right(nullptr), splitAxis(axis) {}
};

// 用于K最近邻搜索的比较器
struct DistanceComparator {
    bool operator()(const std::pair<double, Point*>& a, const std::pair<double, Point*>& b) const {
        return a.first < b.first; // 小顶堆，距离小的优先
    }
};

class KDTree {
private:
    KDNode* root;
    int dimensions;
    
    // 递归构建KD树
    KDNode* buildTree(std::vector<Point*>& points, int depth, int start, int end);
    
    // 移除并行构建KD树方法，改为串行版本
    // KDNode* buildTreeParallel(std::vector<Point*>& points, int depth, int start, int end, int minSizeForParallel = 1000);
    
    // 递归查找最近的点
    void nearestNeighborSearch(KDNode* node, const Point& target, int depth, 
                              Point*& nearest, double& bestDist);
    
    // 递归查找K个最近的点（优化版）
    void kNearestNeighborSearch(KDNode* node, const Point& target, int depth, 
                               std::priority_queue<std::pair<double, Point*>>& nearestPoints, // 修改：改为默认的最大优先队列
                               int k);
    
    // 递归删除KD树
    void deleteTree(KDNode* node);
    
    // 计算指定维度上的方差
    double calculateVariance(const std::vector<Point*>& points, int start, int end, int axis);
    
    // 选择最佳分割维度
    int selectBestSplitAxis(const std::vector<Point*>& points, int start, int end);
    
    // 使用std::nth_element找中位数（保留函数名以兼容现有代码）
    int quickSelect(std::vector<Point*>& points, int start, int end, int k, int axis);
    
    // 检查球面边界是否重叠
    bool sphereBoundsOverlap(const Point& center, double radius, double minBounds[2], double maxBounds[2]);
    
public:
    KDTree();
    ~KDTree();
    
    // 构建KD树
    void build(const std::vector<Point*>& points);
    
    // 查找最近的点
    Point* findNearest(double x, double y);
    
    // 查找K个最近的点
    std::vector<Point*> findKNearest(double x, double y, int k);
    
    // 重新排列点以提高缓存效率
    void reorderPointsForCacheEfficiency(std::vector<Point*>& points);
};

#endif // KDTREE_H