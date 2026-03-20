cmake_minimum_required(VERSION 3.0.2)
project(odev2)

# Find catkin and required components
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  sensor_msgs
  nav_msgs
  geometry_msgs
  std_msgs
  tf
  tf2_ros
  visualization_msgs
  actionlib
  image_transport
  cv_bridge
  angles
)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Catkin package configuration
catkin_package(
  CATKIN_DEPENDS 
    roscpp 
    rospy 
    sensor_msgs 
    nav_msgs 
    geometry_msgs 
    std_msgs 
    tf 
    tf2_ros 
    visualization_msgs
    image_transport
    cv_bridge
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

# Hazmat Detector Node (AKAZE version)
add_executable(hazmat_detector src/hazmat_detector.cpp)
target_link_libraries(hazmat_detector 
  ${catkin_LIBRARIES} 
  ${OpenCV_LIBRARIES}
)



add_executable(qr_detector src/qr_detector.cpp)
target_link_libraries(qr_detector
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
)
add_dependencies(hazmat_detector ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

# Object Detector Node
add_executable(object_detector src/object_detector.cpp)
target_link_libraries(object_detector 
  ${catkin_LIBRARIES} 
  ${OpenCV_LIBRARIES}
)
add_dependencies(object_detector ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

# Explore Node
add_executable(explore_node src/explore.cpp)
target_link_libraries(explore_node ${catkin_LIBRARIES})
add_dependencies(explore_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

# Frontier Explorer Node
add_executable(frontier_explorer src/explore.cpp)
target_link_libraries(frontier_explorer ${catkin_LIBRARIES})

# Trajectory Node
add_executable(trajectory src/trajectory.cpp)
target_link_libraries(trajectory ${catkin_LIBRARIES})

# Install targets
install(TARGETS 
  hazmat_detector
  object_detector
  explore_node 
  frontier_explorer 
  trajectory
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
