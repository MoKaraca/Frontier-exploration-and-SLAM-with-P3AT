# ROS Robot Exploration and Navigation System
# Note: The project isnt finallized so some parts of the project might be incomplete
This is a ROS catkin workspace containing autonomous robot navigation, exploration, and object detection systems. The workspace is designed for simulating and controlling the Pioneer3-AT robot in Gazebo classic and ROS Noetic

## Table of Contents
- [Overview](#overview)
- [Project Structure](#project-structure)
- [Key Components](#key-components)
- [Installation & Setup](#installation--setup)
- [Building](#building)
- [Running the System](#running-the-system)
- [Core Packages](#core-packages)
- [Dependencies](#dependencies)

---

## Overview

This workspace provides a complete ROS-based system for autonomous robots that can:
- **Autonomously explore** unknown environments using frontier-based exploration
- **Navigate safely** using move_base with local/global costmaps
- **Detect objects** including colored barrels, hazmat signs, and QR codes

The primary focus is the `odev2` package, 

---

## Project Structure

```
catkin_ws/src/
├── odev2/                              # Main project package
├── pioneer3at_demo/                    # Pioneer3-AT robot simulation package
├── hector_nist_arenas_gazebo/          # NIST arena environments and elements
│   ├── hector_nist_arena_designer/
│   ├── hector_nist_arena_elements/
│   └── hector_nist_arena_worlds/
└── newModels/                         

---

## Key Components

### 1. **Exploration Module** (`explore.cpp`)
- **Frontier-based autonomous exploration** algorithm A* pathfinding for optimal path planning

### 2. **Object Detection Modules**

#### **Barrel Detector** (`object_detector.cpp`)
- HSV-based color filtering for barrel detection

#### **Hazmat Sign Detector** (`hazmat_detector.cpp`)
- AKAZE feature matching for hazmat sign recognition


#### **QR Code Detector** (`qr.py`)

- QR code detection and decoding, RViz marker visualization of detected QR codes (needs to be manually added in Rviz through the add topic section)

### 3. **Control & Navigation**

#### **Keyboard Teleoperation** (`keyboard.py`)
- Real-time keyboard control of robot velocity (WASD-style and arrow-key movement patterns)
---

## Installation & Setup

### Prerequisites
- Ubuntu 20.04 (ROS Noetic)
- ROS installed: `http://wiki.ros.org/noetic/Installation`
- Gazebo 11+ 
- OpenCV 4.x
- Python 3.8+

### System Dependencies

```bash
# Install required ROS packages
sudo apt-get install ros-noetic-move-base ros-noetic-move-base-msgs \
    ros-noetic-navigation ros-noetic-costmap-2d ros-noetic-base-local-planner \
    ros-noetic-dwa-local-planner ros-noetic-clear-costmap-recovery

# Install OpenCV and vision packages
sudo apt-get install python3-opencv python3-cv-bridge ros-noetic-cv-bridge \
    ros-noetic-image-transport

# Install QR code detection libraries
sudo apt-get install python3-pyzbar libzbar0 libzbar-dev

# Install utilities
sudo apt-get install ros-noetic-gazebo-ros ros-noetic-gazebo-ros-control \
    ros-noetic-tf2-geometry-msgs
```

### Clone & Setup

```bash
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws
git clone https://github.com/MoKaraca/Frontier-exploration-and-SLAM-with-P3AT.git src/

# Source workspace 
echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
echo "source ~/catkin_ws/devel/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

---

## Building

### Build Entire Workspace
```bash
cd ~/catkin_ws
catkin_make
source devel/setup.bash
```

---

## Running the System

### 1. Launch Gazebo with Robot

```bash
# Terminal 1: Start the exploration simulation
roslaunch odev2 exploration.launch

# Optional arguments:
# gui:=true   - Show Gazebo GUI
# verbose:=true - Enable verbose logging
```

### 2. Run Detection Nodes

```bash
# Terminal 2: Start barrel detector
rosrun odev2 object_detector

# Terminal 3: Start hazmat detector
rosrun odev2 hazmat_detector

# Terminal 4: Start QR detector (Python version)
rosrun odev2 qr.py
```

### 3. Teleoperation (Optional)

```bash
# Terminal: Manual control
rosrun odev2 keyboard.py
```

---

## Core Packages

### `odev2` - Main Project Package
**Maintainer:** Dr. Furkan CAKMAK (fcakmak@yildiz.edu.tr)

**Structure:**
- `src/` - C++ source code and algorithms
  - `explore.cpp` - Frontier exploration algorithm
  - `object_detector.cpp` - Barrel detection
  - `hazmat_detector.cpp` - Hazmat sign recognition
  - `qr_detector.cpp` - QR code detection
  - `trajectory.cpp` - Trajectory tracking
  - `HSV_detect/` - Additional CV utilities
- `scripts/` - Python control and utility scripts
  - `keyboard.py` - Teleoperation
  - `qr.py` - QR detection (Python implementation)
- `launch/` - ROS launch files
  - `exploration.launch` - Full system launch configuration
- `nav_params/` - Navigation tuning parameters
  - `base_local_planner_params.yaml` - DWA planner settings
  - `costmap_common_params.yaml` - Shared costmap config
  - `global_costmap_params.yaml` - Global costmap settings
  - `local_costmap_params.yaml` - Local costmap settings
  - `move_base_params.yaml` - Move base parameters
- `rviz/` - RViz configuration files

**Dependencies:**
- roscpp, rospy
- nav_msgs, sensor_msgs, geometry_msgs
- move_base_msgs, actionlib
- cv_bridge, image_transport, opencv

---

### `pioneer3at_demo` - Robot Simulation Package
**State:** Multi-robot simulation demonstration

**Components:**
- `p3at_description/` - URDF robot description and models
- `p3at_gazebo/` - Gazebo simulation world files
- `p3at_plugin/` - Custom Gazebo plugins

**Original Authors:** Yildiz Team (Erkan Uslu, Furkan Cakmak, Muhamet Balcilar, Fatih Amasyali, Sirma Yavuz)

---

### `hector_nist_arenas_gazebo` - Arena & Environment Package
**Purpose:** Provides NIST-compatible challenge environments

**Sub-packages:**
1. **hector_nist_arena_designer** - GUI tools for arena layout design
2. **hector_nist_arena_elements** - 100+ pre-built arena components
3. **hector_nist_arena_worlds** - Pre-configured world files

---

## Dependencies

### Core ROS Packages
```
- roscpp, rospy 
- std_msgs, sensor_msgs, geometry_msgs
- nav_msgs 
- tf, tf2, tf2_ros 
- actionlib 
- move_base_msgs 
- visualization_msgs 
- image_transport, cv_bridge 
```

### External Libraries
```
- OpenCV 4.x 
- pyzbar (Python QR code library)
- Gazebo 11+ 
```

---

## Configuration Parameters

### Navigation Parameters (`nav_params/`)

**Planner Frequency:** Controls how often frontier exploration is computed
```yaml
planner_frequency: 1.0  # Hz
```

**Frontier Size:** Minimum cells in a frontier cluster to be considered
```yaml
min_frontier_size: 33   # cells
```

**Costmap Settings:** Robot inflation radius and observation radius
- Typical inflation radius: 0.2 - 0.3 m
- Observation radius: 5.0 m

---

## Testing & Debugging

### Launch in Debug Mode
```bash
# With verbose logging
roslaunch odev2 exploration.launch verbose:=true

# With Gazebo GUI for monitoring
roslaunch odev2 exploration.launch gui:=true
```

## Contributors

- **Dr. Furkan CAKMAK** - Project Lead (Yildiz Technical University)
- **Yildiz Robotics Team** - Pioneer3-AT simulation base


## References

- [ROS Navigation Wiki](http://wiki.ros.org/navigation)
- [Gazebo Simulation](http://gazebosim.org/)
- [Move Base Documentation](http://wiki.ros.org/move_base)
- [OpenCV Documentation](https://docs.opencv.org/)

---


