#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class HazmatDetector {
private:
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;
    std::vector<cv::Mat> templates_;
    std::vector<std::string> hazmat_names_;
    std::vector<std::vector<cv::KeyPoint>> template_keypoints_;
    std::vector<cv::Mat> template_descriptors_;
    cv::Ptr<cv::Feature2D> detector_;
    cv::Ptr<cv::DescriptorMatcher> matcher_;
    double min_match_ratio_;
    double min_good_matches_;

public:
    HazmatDetector() : it_(nh_), min_match_ratio_(0.7), min_good_matches_(15) {
        // Initialize AKAZE feature detector and matcher
        detector_ = cv::AKAZE::create();
        matcher_ = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMING);
        
        // Subscribe to camera image topic
        image_sub_ = it_.subscribe("/camera/image_raw", 1, &HazmatDetector::imageCallback, this);
        
        // Load templates
        loadTemplates();
        
        ROS_INFO("Hazmat Detector initialized with AKAZE feature matching");
    }

    void loadTemplates() {
        std::vector<std::string> template_files = {
            "blasting-agents.png", "inhalation-hazard.png", "corrosive.png",
            "flammable-gas.png", "dangerous-when-wet.png", "organic-peroxide.png",
            "explosives.png", "oxidizer.png", "oxygen.png",
            "flammable-solid.png", "poison.png", "fuel-oil.png",
            "radioactive.png", "non-flammable-gas.png", "spontaneously-combustible.png"
        };

        hazmat_names_ = {
            "Blasting Agents", "Inhalation Hazard", "Corrosive",
            "Flammable Gas", "Dangerous When Wet", "Organic Peroxide",
            "Explosives", "Oxidizer", "Oxygen",
            "Flammable Solid", "Poison", "Fuel Oil",
            "Radioactive", "Non-Flammable Gas", "Spontaneously Combustible"
        };

        // Load and process each template
        for (size_t i = 0; i < template_files.size(); i++) {
            std::string path = "/home/mo/hazmats/" + template_files[i];
            cv::Mat template_img = cv::imread(path, cv::IMREAD_GRAYSCALE);
            
            if (template_img.empty()) {
                ROS_WARN("Failed to load template: %s", path.c_str());
                continue;
            }
            
            // Detect keypoints and compute descriptors
            std::vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;
            detector_->detectAndCompute(template_img, cv::noArray(), keypoints, descriptors);
            
            if (keypoints.empty()) {
                ROS_WARN("No keypoints found in template: %s", template_files[i].c_str());
                continue;
            }
            
            templates_.push_back(template_img);
            template_keypoints_.push_back(keypoints);
            template_descriptors_.push_back(descriptors);
            
            ROS_INFO("Loaded template: %s (%lu keypoints)", template_files[i].c_str(), keypoints.size());
        }
    }

    void imageCallback(const sensor_msgs::ImageConstPtr& msg) {
        try {
            // Convert ROS image to OpenCV format
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
            cv::Mat scene = cv_ptr->image;
            cv::Mat scene_gray;
            cv::cvtColor(scene, scene_gray, cv::COLOR_BGR2GRAY);
            
            // Process the image
            detectHazmats(scene, scene_gray);
            
            // Display results
            cv::imshow("Hazmat Detection", scene);
            cv::waitKey(1);
        }
        catch (cv_bridge::Exception& e) {
            ROS_ERROR("cv_bridge exception: %s", e.what());
        }
    }

    void detectHazmats(cv::Mat& scene, cv::Mat& scene_gray) {
        if (templates_.empty()) {
            ROS_WARN("No templates loaded for detection");
            return;
        }
        
        // Detect keypoints and descriptors in the scene
        std::vector<cv::KeyPoint> scene_keypoints;
        cv::Mat scene_descriptors;
        detector_->detectAndCompute(scene_gray, cv::noArray(), scene_keypoints, scene_descriptors);
        
        if (scene_keypoints.empty()) {
            ROS_DEBUG("No keypoints found in scene");
            return;
        }
        
        for (size_t i = 0; i < templates_.size(); i++) {
            if (template_descriptors_[i].empty()) continue;
            
            // Match descriptors between template and scene
            std::vector<std::vector<cv::DMatch>> knn_matches;
            matcher_->knnMatch(template_descriptors_[i], scene_descriptors, knn_matches, 2);
            
            // Filter matches using Lowe's ratio test
            std::vector<cv::DMatch> good_matches;
            for (size_t j = 0; j < knn_matches.size(); j++) {
                if (knn_matches[j].size() == 2 && 
                    knn_matches[j][0].distance < min_match_ratio_ * knn_matches[j][1].distance) {
                    good_matches.push_back(knn_matches[j][0]);
                }
            }
            
            // If enough good matches found, estimate homography
            if (good_matches.size() > min_good_matches_) {
                std::vector<cv::Point2f> template_pts;
                std::vector<cv::Point2f> scene_pts;
                
                for (size_t j = 0; j < good_matches.size(); j++) {
                    template_pts.push_back(template_keypoints_[i][good_matches[j].queryIdx].pt);
                    scene_pts.push_back(scene_keypoints[good_matches[j].trainIdx].pt);
                }
                
                // Find homography
                cv::Mat homography = cv::findHomography(template_pts, scene_pts, cv::RANSAC);
                
                if (!homography.empty()) {
                    // Get template corners
                    std::vector<cv::Point2f> template_corners(4);
                    template_corners[0] = cv::Point2f(0, 0);
                    template_corners[1] = cv::Point2f((float)templates_[i].cols, 0);
                    template_corners[2] = cv::Point2f((float)templates_[i].cols, (float)templates_[i].rows);
                    template_corners[3] = cv::Point2f(0, (float)templates_[i].rows);
                    
                    // Transform template corners to scene
                    std::vector<cv::Point2f> scene_corners(4);
                    cv::perspectiveTransform(template_corners, scene_corners, homography);
                    
                    // Draw bounding polygon
                    for (size_t j = 0; j < 4; j++) {
                        cv::line(scene, scene_corners[j], scene_corners[(j+1)%4], cv::Scalar(0, 255, 0), 2);
                    }
                    
                    // Put text label
                    cv::putText(scene, hazmat_names_[i], 
                                scene_corners[0] + cv::Point2f(0, -10), 
                                cv::FONT_HERSHEY_SIMPLEX, 0.5, 
                                cv::Scalar(0, 255, 0), 1);
                    
                    ROS_INFO("Detected %s with %lu good matches", hazmat_names_[i].c_str(), good_matches.size());
                }
            }
        }
    }
};

int main(int argc, char** argv) {
    /*ros::init(argc, argv, "hazmat_detector");
    HazmatDetector detector;
    ros::spin();*/
    return 0;
}
