#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

class QRDetector {
private:
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;
    image_transport::Publisher debug_pub_;
    
    cv::QRCodeDetector qr_detector_;
    
public:
    QRDetector() : it_(nh_) {
        image_sub_ = it_.subscribe("/p3at/camera/rgb/image_raw", 1, &QRDetector::imageCallback, this);
        debug_pub_ = it_.advertise("/debug/qr_detection", 1);
        
        ROS_INFO("QR Detector initialized. Waiting for images...");
    }
    
    void imageCallback(const sensor_msgs::ImageConstPtr& msg) {
        ROS_DEBUG("Received image");
        try {
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
            cv::Mat debug_img = cv_ptr->image.clone();
            
            // Convert to grayscale (QR detection often works better)
            cv::Mat gray;
            cv::cvtColor(cv_ptr->image, gray, cv::COLOR_BGR2GRAY);
            
            std::vector<cv::Point> points;
            std::string decoded = qr_detector_.detectAndDecode(gray, points);
            
            if (!decoded.empty()) {
                ROS_INFO_STREAM("Detected QR: " << decoded);
                
                // Draw detection
                for (int i = 0; i < 4; i++) {
                    cv::line(debug_img, points[i], points[(i+1)%4], cv::Scalar(0,255,0), 4);
                }
                cv::putText(debug_img, decoded, points[0], 
                           cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,0), 2);
            } else {
                ROS_DEBUG("No QR detected in this frame");
            }
            
            // Publish debug image
            sensor_msgs::ImagePtr debug_msg = cv_bridge::CvImage(msg->header, "bgr8", debug_img).toImageMsg();
            debug_pub_.publish(debug_msg);
            
        } catch (cv_bridge::Exception& e) {
            ROS_ERROR("CV Bridge error: %s", e.what());
        } catch (cv::Exception& e) {
            ROS_ERROR("OpenCV error: %s", e.what());
        }
    }
};

int main(int argc, char** argv) {
    /*ros::init(argc, argv, "qr_detector");
    ros::NodeHandle nh;
    
    // Enable debug output
    if (ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug)) {
        ros::console::notifyLoggerLevelsChanged();
    }
    
    QRDetector detector;
    ros::spin();*/
    return 0;
}
