#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <geometry_msgs/PoseStamped.h>
#include <sensor_msgs/LaserScan.h>
#include <nav_msgs/GetMap.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/transform_listener.h>
#include <visualization_msgs/Marker.h>

#include <queue>
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <chrono>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

// ===== OPTIMIZATION CONSTANTS =====
constexpr double SQRT2 = 1.41421356237;
constexpr double FRONTIER_CLUSTERING_DISTANCE = 1.0;  // meters
constexpr double CLUSTER_DISTANCE_THRESHOLD = 1.0;    // meters
constexpr double SCORE_WEIGHT_INFO = 0.33;
constexpr double SCORE_WEIGHT_DISTANCE = 0.6;
constexpr double SCORE_WEIGHT_SIZE = 0.33;

// Score components for frontier selection
struct ScoreWeights {
    double info_gain;
    double proximity;
    double size;
};

struct AStarNode {
    int index;
    double f, g, h;
    int parent;
    
    AStarNode(int i = -1, double f_ = 0, double g_ = 0, double h_ = 0, int p = -1)
        : index(i), f(f_), g(g_), h(h_), parent(p) {}
    
    bool operator>(const AStarNode& other) const {
        return f > other.f;
    }
};

class FrontierExplorer {
private:
    // ===== HELPER METHODS FOR COORDINATE CONVERSION =====
    inline int posToIndex(int x, int y, int width) const {
        return y * width + x;
    }
    
    inline void indexToPos(int idx, int& x, int& y, int width) const {
        x = idx % width;
        y = idx / width;
    }
    
    inline bool isValidIndex(int idx, size_t map_size) const {
        return idx >= 0 && idx < static_cast<int>(map_size);
    }
    
    inline bool isValidCoord(int x, int y, int width, int height) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
    
    // Convert map cell index to world coordinates
    inline geometry_msgs::Point indexToWorld(int idx, int width) const {
        geometry_msgs::Point p;
        int x = idx % width;
        int y = idx / width;
        p.x = x * current_map_.info.resolution + current_map_.info.origin.position.x;
        p.y = y * current_map_.info.resolution + current_map_.info.origin.position.y;
        p.z = 0;
        return p;
    }
    
    // Convert world coordinates to map cell index
    inline int worldToIndex(const geometry_msgs::Point& point, int width) const {
        int x = (point.x - current_map_.info.origin.position.x) / current_map_.info.resolution;
        int y = (point.y - current_map_.info.origin.position.y) / current_map_.info.resolution;
        return posToIndex(x, y, width);
    }

public:
    FrontierExplorer() : 
        tf_listener_(tf_buffer_),
        mb_client_("move_base", true),
        exploration_active_(false),
        min_frontier_size_(20),
        info_radius_(1.0),
        last_map_update_time_(0),
        frontier_visualization_dirty_(true) {
        
        ros::NodeHandle nh;
        ros::NodeHandle private_nh("~");

        private_nh.param("min_frontier_size", min_frontier_size_, 33);
        private_nh.param("info_radius", info_radius_, 1.0);
        private_nh.param("planner_frequency", planner_frequency_, 1.0);
        private_nh.param("visualize", visualize_, true);

        map_sub_ = nh.subscribe("/map", 1, &FrontierExplorer::mapCallback, this);
        scan_sub_ = nh.subscribe("/p3at/scan", 1, &FrontierExplorer::scanCallback, this);

        goal_pub_ = nh.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);
        if (visualize_) {
            marker_pub_ = nh.advertise<visualization_msgs::Marker>("/frontier_markers", 1);
            path_pub_ = nh.advertise<visualization_msgs::Marker>("/astar_path", 1);
        }

        ROS_INFO("Waiting for move_base action server...");
        mb_client_.waitForServer();
        ROS_INFO("Connected to move_base action server");

        exploration_timer_ = nh.createTimer(ros::Duration(1.0 / planner_frequency_),
                                          &FrontierExplorer::explorationTimerCallback, this);
    }

    void mapCallback(const nav_msgs::OccupancyGrid::ConstPtr& msg) {
        current_map_ = *msg;
        map_received_ = true;
        frontier_visualization_dirty_ = true;  // Mark visualization as needing update
        last_map_update_time_ = ros::Time::now().toSec();
    }

    void scanCallback(const sensor_msgs::LaserScan::ConstPtr& msg) {
        current_scan_ = *msg;
    }

    void explorationTimerCallback(const ros::TimerEvent& event) {
        if (!map_received_ || exploration_active_) return;

        std::vector<std::vector<unsigned int>> frontiers = findFrontiers();

        if (frontiers.empty()) {
            ROS_INFO("No frontiers found. Exploration complete?");
            return;
        }

        std::vector<FrontierCluster> clusters = clusterFrontiers(frontiers);

        if (clusters.empty()) {
            ROS_INFO("No valid frontier clusters found");
            return;
        }

        FrontierCluster best_frontier = selectBestFrontier(clusters);
        sendExplorationGoal(best_frontier.centroid);
    }

private:
    struct FrontierCluster {
        std::vector<unsigned int> points;
        geometry_msgs::Point centroid;
        double size;
        double min_distance;
        double information_gain;
    };

    std::vector<std::vector<unsigned int>> findFrontiers() {
        std::vector<std::vector<unsigned int>> frontiers;
        std::set<unsigned int> visited;

        int width = current_map_.info.width;
        int height = current_map_.info.height;

        // 8-directional neighbor offsets
        const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
        const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

        for (unsigned int idx = 0; idx < current_map_.data.size(); ++idx) {
            // Skip if already visited or not free
            if (current_map_.data[idx] != 0 || visited.count(idx) > 0) {
                continue;
            }

            // Check if this cell is on frontier (has unknown neighbor)
            int x, y;
            indexToPos(idx, x, y, width);
            
            bool is_frontier = false;
            for (int dir = 0; dir < 8; ++dir) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                
                if (!isValidCoord(nx, ny, width, height)) continue;
                
                int neighbor_idx = posToIndex(nx, ny, width);
                if (current_map_.data[neighbor_idx] == -1) {
                    is_frontier = true;
                    break;
                }
            }

            if (!is_frontier) continue;

            // BFS to find all connected frontier cells
            std::vector<unsigned int> frontier;
            std::queue<unsigned int> queue;
            queue.push(idx);
            visited.insert(idx);

            while (!queue.empty()) {
                unsigned int cell = queue.front();
                queue.pop();
                frontier.push_back(cell);

                int cx, cy;
                indexToPos(cell, cx, cy, width);

                // Check all 8 neighbors
                for (int dir = 0; dir < 8; ++dir) {
                    int nx = cx + dx[dir];
                    int ny = cy + dy[dir];

                    if (!isValidCoord(nx, ny, width, height)) continue;

                    int neighbor_idx = posToIndex(nx, ny, width);
                    
                    // Skip if already visited or occupied
                    if (visited.count(neighbor_idx) > 0 || current_map_.data[neighbor_idx] != 0) {
                        continue;
                    }

                    // Check if neighbor is on frontier
                    bool neighbor_is_frontier = false;
                    for (int dir2 = 0; dir2 < 8; ++dir2) {
                        int nnx = nx + dx[dir2];
                        int nny = ny + dy[dir2];
                        
                        if (!isValidCoord(nnx, nny, width, height)) continue;
                        
                        int nneighbor_idx = posToIndex(nnx, nny, width);
                        if (current_map_.data[nneighbor_idx] == -1) {
                            neighbor_is_frontier = true;
                            break;
                        }
                    }

                    if (neighbor_is_frontier) {
                        queue.push(neighbor_idx);
                        visited.insert(neighbor_idx);
                    }
                }
            }

            // Only keep frontiers with sufficient size
            if (frontier.size() >= static_cast<size_t>(min_frontier_size_)) {
                frontiers.push_back(std::move(frontier));
            }
        }

        ROS_DEBUG("Found %zu frontier clusters", frontiers.size());
        return frontiers;
    }

    std::vector<FrontierCluster> clusterFrontiers(const std::vector<std::vector<unsigned int>>& frontiers) {
        std::vector<FrontierCluster> clusters;
        if (frontiers.empty()) return clusters;

        double resolution = current_map_.info.resolution;
        int cluster_dist_threshold = static_cast<int>(CLUSTER_DISTANCE_THRESHOLD / resolution);

        for (const auto& frontier : frontiers) {
            FrontierCluster new_cluster;
            
            // Calculate centroid first
            double x_sum = 0, y_sum = 0;
            for (unsigned int cell : frontier) {
                int x = cell % current_map_.info.width;
                int y = cell / current_map_.info.width;
                x_sum += x;
                y_sum += y;
            }
            
            int centroid_x = static_cast<int>(x_sum / frontier.size());
            int centroid_y = static_cast<int>(y_sum / frontier.size());
            
            // Check if we can merge with existing cluster
            bool merged = false;
            for (auto& cluster : clusters) {
                int cluster_cx = static_cast<int>(cluster.centroid.x / resolution);
                int cluster_cy = static_cast<int>(cluster.centroid.y / resolution);
                
                int dist = static_cast<int>(std::hypot(centroid_x - cluster_cx, centroid_y - cluster_cy));
                
                if (dist < cluster_dist_threshold) {
                    // Merge: recalculate centroid for merged cluster
                    size_t old_size = cluster.points.size();
                    cluster.points.insert(cluster.points.end(), frontier.begin(), frontier.end());
                    
                    // Update centroid with weighted average
                    double old_weight = static_cast<double>(old_size);
                    double new_weight = frontier.size();
                    double total_weight = old_weight + new_weight;
                    
                    cluster.centroid.x = (cluster.centroid.x * old_weight + centroid_x * new_weight * resolution) / total_weight;
                    cluster.centroid.y = (cluster.centroid.y * old_weight + centroid_y * new_weight * resolution) / total_weight;
                    
                    merged = true;
                    break;
                }
            }

            if (!merged) {
                new_cluster.points = frontier;
                new_cluster.centroid.x = centroid_x * resolution + current_map_.info.origin.position.x;
                new_cluster.centroid.y = centroid_y * resolution + current_map_.info.origin.position.y;
                new_cluster.centroid.z = 0;
                clusters.push_back(std::move(new_cluster));
            }
        }

        // Compute metrics for each cluster (do this after all merging)
        for (auto& cluster : clusters) {
            cluster.size = cluster.points.size() * resolution * resolution;
            
            // Get robot position
            geometry_msgs::TransformStamped transform;
            try {
                transform = tf_buffer_.lookupTransform("map", "p3at/base_link", ros::Time(0));
                double robot_x = transform.transform.translation.x;
                double robot_y = transform.transform.translation.y;
                
                cluster.min_distance = std::hypot(cluster.centroid.x - robot_x, 
                                                 cluster.centroid.y - robot_y);
            } catch (tf2::TransformException& ex) {
                ROS_DEBUG("TF lookup error: %s - using max distance", ex.what());
                cluster.min_distance = std::numeric_limits<double>::max();
            }
            
            // Calculate information gain only once per cluster
            cluster.information_gain = calculateInformationGain(cluster.centroid);
        }

        return clusters;
    }

    double calculateInformationGain(const geometry_msgs::Point& point) {
        double gain = 0;
        int width = current_map_.info.width;
        int height = current_map_.info.height;
        double resolution = current_map_.info.resolution;
        
        int x_center = static_cast<int>((point.x - current_map_.info.origin.position.x) / resolution);
        int y_center = static_cast<int>((point.y - current_map_.info.origin.position.y) / resolution);
        int radius_cells = static_cast<int>(info_radius_ / resolution);
        
        // Clamp to map boundaries
        int x_min = std::max(0, x_center - radius_cells);
        int x_max = std::min(width - 1, x_center + radius_cells);
        int y_min = std::max(0, y_center - radius_cells);
        int y_max = std::min(height - 1, y_center + radius_cells);
        
        // Count unknown cells in info radius
        for (int x = x_min; x <= x_max; ++x) {
            for (int y = y_min; y <= y_max; ++y) {
                if (current_map_.data[posToIndex(x, y, width)] == -1) {
                    gain += 1.0;
                }
            }
        }
        
        return gain * resolution * resolution;
    }

    FrontierCluster selectBestFrontier(const std::vector<FrontierCluster>& clusters) {
        FrontierCluster best;
        double best_score = -std::numeric_limits<double>::max();
        
        for (const auto& cluster : clusters) {
            // Multi-objective scoring:
            // - Information gain: how much unknown area can be revealed
            // - Proximity: closer frontiers are preferred
            // - Size: larger frontier clusters are more stable
            
            double distance_score = 1.0 / (1.0 + cluster.min_distance);
            double size_score = cluster.size / 10.0;
            
            double score = cluster.information_gain * SCORE_WEIGHT_INFO + 
                          distance_score * SCORE_WEIGHT_DISTANCE +
                          size_score * SCORE_WEIGHT_SIZE;
            
            if (score > best_score) {
                best_score = score;
                best = cluster;
            }
        }
        
        ROS_INFO("Selected frontier with score %.2f (size: %.2f m2, dist: %.2f m, info: %.2f m2)",
                best_score, best.size, best.min_distance, best.information_gain);
        
        return best;
    }

    void sendExplorationGoal(const geometry_msgs::Point& target) {
        exploration_active_ = true;
        
        geometry_msgs::PoseStamped goal;
        goal.header.frame_id = "map";
        goal.header.stamp = ros::Time::now();
        goal.pose.position = target;
        
        // Get current robot position for proper yaw calculation
        geometry_msgs::TransformStamped transform;
        double yaw = 0;
        
        try {
            transform = tf_buffer_.lookupTransform("map", "p3at/base_link", ros::Time(0));
            double robot_x = transform.transform.translation.x;
            double robot_y = transform.transform.translation.y;
            
            // Calculate yaw from robot to target (FIX: was using map origin before)
            yaw = atan2(target.y - robot_y, target.x - robot_x);
        } catch (tf2::TransformException& ex) {
            ROS_WARN("Could not get robot transform for goal yaw calculation: %s", ex.what());
            yaw = 0;
        }
        
        goal.pose.orientation = tf::createQuaternionMsgFromYaw(yaw);
        
        goal_pub_.publish(goal);
        
        move_base_msgs::MoveBaseGoal mb_goal;
        mb_goal.target_pose = goal;
        
        mb_client_.sendGoal(mb_goal, 
                          boost::bind(&FrontierExplorer::goalDoneCallback, this, _1, _2),
                          MoveBaseClient::SimpleActiveCallback(),
                          boost::bind(&FrontierExplorer::goalFeedbackCallback, this, _1));
        
        ROS_INFO("Sent exploration goal to (%.2f, %.2f) with yaw %.2f rad", target.x, target.y, yaw);
        
        // Only visualize if enabled and if map has been updated
        if (visualize_) {
            if (frontier_visualization_dirty_) {
                visualizeFrontiers();
                frontier_visualization_dirty_ = false;
            }
            
            try {
                geometry_msgs::TransformStamped robot_transform = tf_buffer_.lookupTransform("map", "p3at/base_link", ros::Time(0));
                geometry_msgs::Point start;
                start.x = robot_transform.transform.translation.x;
                start.y = robot_transform.transform.translation.y;
                start.z = 0;
                visualizeAStarPath(start, target);
            } catch (tf2::TransformException& ex) {
                ROS_DEBUG("Could not visualize A* path: %s", ex.what());
            }
        }
    }

    void goalDoneCallback(const actionlib::SimpleClientGoalState& state,
                         const move_base_msgs::MoveBaseResultConstPtr& result) {
        ROS_INFO("Exploration goal finished: %s", state.toString().c_str());
        exploration_active_ = false;
        
        // Log goal outcome for diagnostics
        if (state == actionlib::SimpleClientGoalState::SUCCEEDED) {
            ROS_INFO("Goal reached successfully");
        } else if (state == actionlib::SimpleClientGoalState::ABORTED) {
            ROS_WARN("Goal was aborted - may need to select new frontier");
        } else {
            ROS_WARN("Goal ended with state: %s", state.toString().c_str());
        }
    }

    void goalFeedbackCallback(const move_base_msgs::MoveBaseFeedbackConstPtr& feedback) {
        // Feedback callback - can be used for intermediate progress checks
        // Currently not used, but useful for real-time progress monitoring
    }

    void visualizeFrontiers() {
        visualization_msgs::Marker marker;
        marker.header.frame_id = "map";
        marker.header.stamp = ros::Time::now();
        marker.ns = "frontiers";
        marker.id = 0;
        marker.type = visualization_msgs::Marker::POINTS;
        marker.action = visualization_msgs::Marker::ADD;
        marker.pose.orientation.w = 1.0;
        marker.scale.x = 0.1;
        marker.scale.y = 0.1;
        marker.color.r = 1.0;
        marker.color.a = 1.0;
        
        int width = current_map_.info.width;
        double resolution = current_map_.info.resolution;
        
        // Reserve space to avoid repeated allocations
        marker.points.reserve(current_map_.data.size() / 10);  // Estimate: ~10% are unknowns
        
        // Only include truly unknown cells
        for (unsigned int idx = 0; idx < current_map_.data.size(); ++idx) {
            if (current_map_.data[idx] == -1) {
                marker.points.push_back(indexToWorld(idx, width));
            }
        }
        
        marker_pub_.publish(marker);
        ROS_DEBUG("Published frontier visualization with %zu points", marker.points.size());
    }

    std::vector<geometry_msgs::Point> findAStarPath(const geometry_msgs::Point& start, const geometry_msgs::Point& goal) {
        std::vector<geometry_msgs::Point> path;
        
        int width = current_map_.info.width;
        int height = current_map_.info.height;
        
        int start_idx = worldToIndex(start, width);
        int goal_idx = worldToIndex(goal, width);
        
        int start_x, start_y, goal_x, goal_y;
        indexToPos(start_idx, start_x, start_y, width);
        indexToPos(goal_idx, goal_x, goal_y, width);
        
        // Validate positions
        if (!isValidCoord(start_x, start_y, width, height) || current_map_.data[start_idx] > 0) {
            ROS_DEBUG("Start position (%.2f, %.2f) is invalid or in obstacle", start.x, start.y);
            return path;
        }
        
        if (!isValidCoord(goal_x, goal_y, width, height) || current_map_.data[goal_idx] > 0) {
            ROS_DEBUG("Goal position (%.2f, %.2f) is invalid or in obstacle", goal.x, goal.y);
            return path;
        }

        std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> open_set;
        std::unordered_map<int, double> g_scores;
        std::unordered_map<int, int> came_from;
        
        double initial_h = heuristic(start_idx, goal_idx);
        open_set.emplace(start_idx, initial_h, 0, initial_h);
        g_scores[start_idx] = 0;
        
        const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
        const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        const double costs[8] = {SQRT2, 1.0, SQRT2, 1.0, 1.0, SQRT2, 1.0, SQRT2};
        
        int iterations = 0;
        const int MAX_ITERATIONS = 100000;  // Prevent infinite loops
        
        while (!open_set.empty() && iterations < MAX_ITERATIONS) {
            ++iterations;
            
            AStarNode current = open_set.top();
            open_set.pop();
            
            if (current.index == goal_idx) {
                path = reconstructPath(came_from, current.index, start_idx);
                ROS_DEBUG("A* found path with %zu waypoints in %d iterations", path.size(), iterations);
                return path;
            }
            
            int x, y;
            indexToPos(current.index, x, y, width);
            
            for (int i = 0; i < 8; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                
                if (!isValidCoord(nx, ny, width, height)) continue;
                
                int neighbor_idx = posToIndex(nx, ny, width);
                if (current_map_.data[neighbor_idx] > 0) continue;  // Skip occupied cells
                
                double tentative_g = g_scores[current.index] + costs[i];
                
                // Only process if we found a better path
                if (g_scores.find(neighbor_idx) == g_scores.end() || tentative_g < g_scores[neighbor_idx]) {
                    came_from[neighbor_idx] = current.index;
                    g_scores[neighbor_idx] = tentative_g;
                    double h = heuristic(neighbor_idx, goal_idx);
                    double f = tentative_g + h;
                    open_set.emplace(neighbor_idx, f, tentative_g, h, current.index);
                }
            }
        }
        
        if (iterations >= MAX_ITERATIONS) {
            ROS_WARN("A* exceeded maximum iterations (%d) without finding path", MAX_ITERATIONS);
        } else {
            ROS_DEBUG("A* failed to find a path to goal");
        }
        return path;
    }
    
    double heuristic(int a, int b) const {
        int ax, ay, bx, by;
        indexToPos(a, ax, ay, current_map_.info.width);
        indexToPos(b, bx, by, current_map_.info.width);
        
        // Euclidean distance heuristic
        return std::hypot(ax - bx, ay - by);
    }
    
    std::vector<geometry_msgs::Point> reconstructPath(const std::unordered_map<int, int>& came_from, int current, int start) {
        std::vector<int> indices;
        indices.reserve(100);  // Typical path length
        indices.push_back(current);
        
        while (came_from.find(current) != came_from.end()) {
            current = came_from.at(current);
            indices.push_back(current);
        }
        
        std::reverse(indices.begin(), indices.end());
        
        std::vector<geometry_msgs::Point> path;
        path.reserve(indices.size());
        for (int idx : indices) {
            path.push_back(indexToWorld(idx, current_map_.info.width));
        }
        
        return path;
    }
    
    void visualizeAStarPath(const geometry_msgs::Point& start, const geometry_msgs::Point& goal) {
        std::vector<geometry_msgs::Point> path = findAStarPath(start, goal);
        
        if (path.empty()) {
            ROS_DEBUG("Skipping A* path visualization - no path found");
            return;
        }
        
        visualization_msgs::Marker path_marker;
        path_marker.header.frame_id = "map";
        path_marker.header.stamp = ros::Time::now();
        path_marker.ns = "astar_path";
        path_marker.id = 0;
        path_marker.type = visualization_msgs::Marker::LINE_STRIP;
        path_marker.action = visualization_msgs::Marker::ADD;
        path_marker.pose.orientation.w = 1.0;
        path_marker.scale.x = 0.1;
        path_marker.color.g = 1.0;
        path_marker.color.a = 1.0;
        path_marker.points = path;
        
        path_pub_.publish(path_marker);
    }

    // ===== ROS COMMUNICATION =====
    ros::Subscriber map_sub_;
    ros::Subscriber scan_sub_;
    ros::Publisher goal_pub_;
    ros::Publisher marker_pub_;
    ros::Publisher path_pub_;
    tf2_ros::Buffer tf_buffer_;
    tf2_ros::TransformListener tf_listener_;
    MoveBaseClient mb_client_;
    ros::Timer exploration_timer_;

    // ===== MAP DATA =====
    nav_msgs::OccupancyGrid current_map_;
    sensor_msgs::LaserScan current_scan_;
    bool map_received_ = false;
    bool exploration_active_;
    
    // ===== OPTIMIZATION: CACHING & DIRTY FLAGS =====
    double last_map_update_time_;      // Timestamp of last map update
    bool frontier_visualization_dirty_; // Flag to indicate frontier viz needs update
    
    // ===== ROS PARAMETERS =====
    double planner_frequency_;
    bool visualize_;
    int min_frontier_size_;
    double info_radius_;
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "frontier_explorer");
    FrontierExplorer explorer;
    ros::spin();
    return 0;
}
