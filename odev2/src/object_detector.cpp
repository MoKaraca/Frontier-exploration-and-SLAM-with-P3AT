#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/opencv.hpp>
#include <vector>

// Define HSV color ranges for barrel detection (Hue, Saturation, Value)
const cv::Scalar PURPLEISH_LOWER = cv::Scalar(145, 60, 130);    // HSV values
const cv::Scalar PURPLEISH_UPPER = cv::Scalar(170, 90, 160);  // HSV values
const cv::Scalar LIGHT_GREENISH_LOWER = cv::Scalar(8, 175, 83);  // HSV values
const cv::Scalar LIGHT_GREENISH_UPPER = cv::Scalar(24, 205, 115); // HSV values
// Add your new barrel color HSV range here:
const cv::Scalar NEW_COLOR_LOWER = cv::Scalar(3, 90, 125);    // Replace with your HSV values
const cv::Scalar NEW_COLOR_UPPER = cv::Scalar(18, 130, 180);    // Replace with your HSV values

// Minimum size for a barrel cluster (adjust based on your camera resolution)
const int MIN_BARREL_PIXELS = 1500;

void detectBarrels(const cv::Mat& hsv_image, cv::Mat& output_image) {
    // Create masks for our target colors in HSV space
    cv::Mat purple_mask, green_mask, new_color_mask;
    cv::inRange(hsv_image, PURPLEISH_LOWER, PURPLEISH_UPPER, purple_mask);
    cv::inRange(hsv_image, LIGHT_GREENISH_LOWER, LIGHT_GREENISH_UPPER, green_mask);
    cv::inRange(hsv_image, NEW_COLOR_LOWER, NEW_COLOR_UPPER, new_color_mask);
    
    // Combine masks
    cv::Mat combined_mask = purple_mask | green_mask | new_color_mask;
    
    // Find contours in the combined mask
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combined_mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Process each contour
    for (const auto& contour : contours) {
        // Filter small contours
        if (cv::contourArea(contour) < MIN_BARREL_PIXELS) {
            continue;
        }
        
        // Get the bounding rectangle
        cv::Rect boundRect = cv::boundingRect(contour);
        
        // Create ROI in HSV space for color verification
        cv::Mat hsv_roi = hsv_image(boundRect);
        
        // Create masks for the ROI in HSV space
        cv::Mat roi_purple_mask, roi_green_mask, roi_new_color_mask;
        cv::inRange(hsv_roi, PURPLEISH_LOWER, PURPLEISH_UPPER, roi_purple_mask);
        cv::inRange(hsv_roi, LIGHT_GREENISH_LOWER, LIGHT_GREENISH_UPPER, roi_green_mask);
        cv::inRange(hsv_roi, NEW_COLOR_LOWER, NEW_COLOR_UPPER, roi_new_color_mask);
        
        // Count pixels in HSV space
        int purple_pixels = cv::countNonZero(roi_purple_mask);
        int green_pixels = cv::countNonZero(roi_green_mask);
        int new_color_pixels = cv::countNonZero(roi_new_color_mask);
        
        // Label the barrel
        std::string label;
        cv::Scalar text_color;
        
        if (purple_pixels > green_pixels && purple_pixels > new_color_pixels) {
            label = "Purpleish Barrel";
            text_color = cv::Scalar(255, 0, 255); // Purple in BGR
        } 
        else if (green_pixels > purple_pixels && green_pixels > new_color_pixels) {
            label = "Light Greenish Barrel";
            text_color = cv::Scalar(0, 255, 0); // Green in BGR
        }
        else {
            label = "New Color Barrel";
            text_color = cv::Scalar(0, 165, 255); // Orange in BGR (example for new color)
        }
        
        // Draw the bounding box and label (on the output BGR image)
        cv::rectangle(output_image, boundRect.tl(), boundRect.br(), text_color, 2);
        cv::putText(output_image, label, boundRect.tl() - cv::Point(0, 10),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, text_color, 1);
        
        // Optional: Print HSV stats for debugging
        ROS_INFO_STREAM("Detected " << label << " with " << purple_pixels 
                        << " purple pixels, " << green_pixels << " green pixels, and "
                        << new_color_pixels << " new color pixels");
    }
}

void imageCallback(const sensor_msgs::ImageConstPtr& msg) {
    try {
        // Convert the ROS image to an OpenCV BGR image
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        
        // Create output image with barrel annotations (in BGR space for display)
        cv::Mat output_image = cv_ptr->image.clone();
        
        // Convert to HSV for processing
        cv::Mat hsv_image;
        cv::cvtColor(cv_ptr->image, hsv_image, cv::COLOR_BGR2HSV);
        
        // Detect barrels using HSV image
        detectBarrels(hsv_image, output_image);
        
        // Show the images
        cv::imshow("Original BGR Image", cv_ptr->image);
        cv::imshow("HSV Image", hsv_image);
        cv::imshow("Barrel Detection", output_image);
        cv::waitKey(1);
    } catch (cv_bridge::Exception& e) {
        ROS_ERROR("cv_bridge exception: %s", e.what());
    }
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "barrel_detector");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);

    // Subscribe to the RGB image topic
    image_transport::Subscriber sub = it.subscribe("/p3at/camera/rgb/image_raw", 1, imageCallback);

    ros::spin();
    return 0;
}
