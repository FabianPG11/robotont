#!/usr/bin/env python
import rospy
from geometry_msgs.msg import Twist
import time

velocity_publisher = rospy.Publisher(
    '/robotont/cmd_vel', Twist, queue_size=10)
vel_msg = Twist()
#######################
# YOUR FUNCTIONS HERE #
#######################


###########################
# YOUR FUNCTIONS HERE END #
###########################

def move():
    # Starts a new node
    rospy.init_node('robotont_velocity_publisher', anonymous=True)

    vel_msg.linear.x = 0
    vel_msg.linear.y = 0
    vel_msg.linear.z = 0
    vel_msg.angular.x = 0
    vel_msg.angular.y = 0
    vel_msg.angular.z = 0

    while not rospy.is_shutdown():

        try:
            ########################
            # YOUR CODE HERE START #
            ########################
            vel_msg.linear.x = 0
            vel_msg.linear.y = 0
            vel_msg.angular.z = 0
            velocity_publisher.publish(vel_msg)
            rospy.sleep(0.1)
            ######################
            # YOUR CODE HERE END #
            ######################

        except KeyboardInterrupt:
            # After the loop, stops the robot
            vel_msg.linear.x = 0
            vel_msg.linear.y = 0
            vel_msg.linear.z = 0
            vel_msg.angular.x = 0
            vel_msg.angular.y = 0
            vel_msg.angular.z = 0
            # Force the robot to stop
            velocity_publisher.publish(vel_msg)
            rospy.sleep(1)


if __name__ == '__main__':
    try:
        move()
    except rospy.ROSInterruptException:
        pass
