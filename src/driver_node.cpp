#include "ros/ros.h"
#include "std_msgs/String.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include <sstream>

// Global Variable Definitions
geometry_msgs::Twist rqt_vel; //Stores rqt input
geometry_msgs::Twist cmd_vel; //Stores actual velocity
sensor_msgs::LaserScan laser; //Stores laser data
bool robot_check = false; //Checks if a robot with LIDAR has been made

void desiredVelCallback(const geometry_msgs::Twist::ConstPtr& msg)
{
  rqt_vel.linear = msg->linear; //gets linear velocity from rqt
  rqt_vel.angular = msg->angular; //gets angular velocity from rqt
  ROS_INFO("Forward velocity %2.2f; Angular velocity %2.2f", msg->linear.x, msg->angular.z);
}

void laserScanCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
  robot_check = true; //Sets that there is a robot with LIDAR to true
  laser.ranges = msg->ranges; //Sets laser ranges received from robot
}

int main(int argc, char **argv)
{
    // Initialize
    ros::init(argc, argv, "stdr_driver_node");

    // Initialize the node
    ros::NodeHandle n;

    // Initialize the Subscriber
    ros::Subscriber sub = n.subscribe("des_vel", 1000, desiredVelCallback);
    ros::Subscriber sub1 = n.subscribe("laser_1", 1000, laserScanCallback);

    // Initialize the Publisher
    ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

    // Sets frequency
    ros::Rate loop_rate(10);

    int count = 0;
    //If true, stops the robot's forward movement
    bool stop = false;
  while (ros::ok())
  {
    //Sets actual velocity to rqt input velocity
    cmd_vel = rqt_vel;
    stop = false;
  
  //Checks if there is a robot
  if(robot_check){
    //Checks lasers 125-155 distance from wall. If <.5 meters, sets stop to true.
    for(int i=125; i<155; i++){
        if(laser.ranges[i] < 0.5){
            stop=true;
        }
    }

    //If stop is true and rqt input velocity is greater than 0, stops the robot and issues a warning.
    if(stop){
        cmd_vel.linear.x = 0;
        ROS_WARN("Too close to wall. Full Stop Initiated.");
    }
    
    //If everything is working, no warnings given.
    else{
      ROS_INFO("No Warnings.");
    }
    
    //Publishes cmd_vel
    cmd_vel_pub.publish(cmd_vel);
  }

  //If there is no robot with LIDAR, sends a fatal error message.
  else if(!robot_check){
    ROS_FATAL("No Robot with LIDAR found.");
  }
  
  ros::spinOnce();
  loop_rate.sleep();
  ++count;
  }
  return 0;
}