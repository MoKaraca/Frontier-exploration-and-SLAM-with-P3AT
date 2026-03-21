#!/usr/bin/env python3
import rospy
import cv2
import numpy as np
from cv_bridge import CvBridge
from sensor_msgs.msg import Image
from std_msgs.msg import String
from pyzbar.pyzbar import decode

import tf2_ros
import geometry_msgs.msg
from tf2_geometry_msgs.tf2_geometry_msgs import do_transform_point
from geometry_msgs.msg import PointStamped
from visualization_msgs.msg import Marker

class QRCodeDetectorNode:
    def __init__(self):
        rospy.init_node('qr_code_detector_node')
        self.bridge = CvBridge()

        # Subscribe to the RGB image topic
        self.image_sub = rospy.Subscriber('/p3at/camera/rgb/image_raw', Image, self.image_callback)

        # Publisher for QR code string data
        self.qr_pub = rospy.Publisher('/detected_qr_codes', String, queue_size=10)

        # TF buffer and listener to get robot pose in map frame
        self.tf_buffer = tf2_ros.Buffer()
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer)

        # Marker publisher for RViz
        self.marker_pub = rospy.Publisher('/qr_code_markers', Marker, queue_size=10)

        rospy.loginfo("QR Code Detector Node Initialized")

    def publish_marker(self, text, position, marker_id):
        marker = Marker()
        marker.header.frame_id = "map"
        marker.header.stamp = rospy.Time.now()
        marker.ns = "qr_codes"
        marker.id = marker_id
        marker.type = Marker.TEXT_VIEW_FACING
        marker.action = Marker.ADD
        marker.pose.position.x = position.x
        marker.pose.position.y = position.y
        marker.pose.position.z = 1.0  # Slightly above the ground
        marker.pose.orientation.w = 1.0
        marker.scale.z = 0.4  # Text height
        marker.color.a = 1.0
        marker.color.r = 1.0
        marker.color.g = 1.0
        marker.color.b = 0.0
        marker.text = f"qr_code, value: {text}"

        self.marker_pub.publish(marker)

    def image_callback(self, msg):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        except Exception as e:
            rospy.logerr("CV Bridge error: %s", str(e))
            return

        gray = cv2.cvtColor(cv_image, cv2.COLOR_BGR2GRAY)
        decoded_objects = decode(gray)

        for obj in decoded_objects:
            data = obj.data.decode('utf-8')
            rospy.loginfo(f"QR Code Detected: {data}")
            self.qr_pub.publish(data)

            # Draw bounding box
            points = obj.polygon
            if len(points) > 4:
                hull = cv2.convexHull(np.array([point for point in points], dtype=np.float32))
                points = hull
            else:
                points = np.array(points, dtype=np.int32)

            n = len(points)
            for i in range(n):
                cv2.line(cv_image, tuple(points[i]), tuple(points[(i + 1) % n]), (0, 255, 0), 2)

            if len(points) > 0:
                top_left = tuple(points[0])
                cv2.putText(cv_image, data, (top_left[0], top_left[1] - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2)

            # Estimate QR position 1 meter in front of camera
            try:
                transform = self.tf_buffer.lookup_transform("map", msg.header.frame_id, rospy.Time(0), rospy.Duration(1.0))

                qr_camera_point = PointStamped()
                qr_camera_point.header.stamp = rospy.Time(0)
                qr_camera_point.header.frame_id = msg.header.frame_id
                qr_camera_point.point.x = 1.0  # 1 meter ahead
                qr_camera_point.point.y = 0.0
                qr_camera_point.point.z = 0.0

                qr_map_point = do_transform_point(qr_camera_point, transform)

                # Publish RViz marker
                marker_id = abs(hash(data)) % 10000
                self.publish_marker(data, qr_map_point.point, marker_id)

            except Exception as e:
                rospy.logwarn("TF transform failed: %s", str(e))

        # Show annotated feed
        cv2.imshow("QR Code Detection", cv_image)
        cv2.waitKey(1)

if __name__ == '__main__':
    try:
        node = QRCodeDetectorNode()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass

