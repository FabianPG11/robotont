/* 
* This node listens to teleop (cmd_vel), then formats the command accordingly 
* and publishes the formatted string to serial (topic:'serial_write')
*/
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <string>
#include <cmath>

// The String that contains the command that is being broadcasted to hardware
//std::string output_cmd = "0:0:0;\r\n";
std::string output_cmd = "\27\r\n"; //send ESC, which means stop

int wheelAmount;

// initial list for the angles of the wheels
std::vector<float> wheelAngles;

//omniwheel data
float wheelDistanceFromCenter;
float wheelRadius;
float gearboxReductionRatio;
int encoderEdgesPerMotorRevolution;
float pidControlFrequency;

float wheelSpeedToMainboardUnits;

float wheel_ang_scaler = 0.5;
float angular_vel_scaler = 2;

void format_cmd(const geometry_msgs::Twist& cmd_vel_msg){

    float m0 = 0;
    float m1 = 0;
    float m2 = 0;

    float vel_x = cmd_vel_msg.linear.x;
    float vel_y = -cmd_vel_msg.linear.y;
    float vel_t = cmd_vel_msg.angular.z;

    int motorCount = wheelAmount;

    float robotSpeed = 0;
    
    if (vel_x == 0){
		robotSpeed = std::abs(vel_y);
    }
    else if (vel_y == 0){
		robotSpeed = std::abs(vel_x);
    }
    else{
        robotSpeed = sqrt(vel_x*vel_x + vel_y*vel_y);
    }   

    float robotDirectionAngle = atan2(vel_x, vel_y);	

    //ROS_INFO_STREAM("robotDirectionAngle: " << (float)wheelAngles[0]); //debug purpouse

    float robotAngularVelocity = -vel_t * angular_vel_scaler;

    float wheelLinearVelocity[motorCount];
    float wheelAngularSpeedMainboardUnits[motorCount];

    for(int i= 0; i < motorCount; i++){
		wheelLinearVelocity[i] = robotSpeed * cos(robotDirectionAngle - wheelAngles[i]) + wheelDistanceFromCenter * robotAngularVelocity;
		wheelAngularSpeedMainboardUnits[i] = wheelLinearVelocity[i] * wheelSpeedToMainboardUnits;
    }


    if (vel_x == 0 and vel_y == 0 and vel_t == 0){
		m0 = m1 = m2 = 0;
		output_cmd = "\x1B\r\n"; //send ESC, which means stop
    }
    else {
		m0 = wheelAngularSpeedMainboardUnits[0] * wheel_ang_scaler;
		m1 = wheelAngularSpeedMainboardUnits[1] * wheel_ang_scaler;
		m2 = wheelAngularSpeedMainboardUnits[2] * wheel_ang_scaler;
		std::stringstream sstm;
		//    sstm << 'a' << static_cast<int>(m0) << 'b' << static_cast<int>(m1) << 'c' << static_cast<int>(m2) << '\n'; // for previous firmware implementation
		sstm << static_cast<int>(m0) << ':' << static_cast<int>(m1) << ':' << static_cast<int>(m2) << '\r' <<'\n';
		//    ROS_INFO_STREAM("I calculated: \n" << sstm.str()); // debug purpouse

		output_cmd = sstm.str(); // send m0:m1:m2(LineFeed)(CarriageReturn)
    }
    // http://stackoverflow.com/questions/191757/how-to-concatenate-a-stdstring-and-an-int 
    // important note about speed
}


/*
* SUBSCRIBER CALLBACK FUNCTION
* The function starts everytime there is a new message published to given topic
*/
void teleop_callback(const geometry_msgs::Twist& cmd_vel_msg){
    ROS_INFO_STREAM("I heard: \r\n" << cmd_vel_msg); // cmd_vel input
    format_cmd(cmd_vel_msg);
    ROS_INFO_STREAM("I will publish: \r\n" << output_cmd); // "m0:m1:m2\n" output
}


/*
* THE MAIN FUNCTION
* the function initialises the node and creates a subscriber and a publisher
* Then as long as the program is running, it will publish the velocity command 
* to a given topic which will forward everything to harware through serial 
* connection
*/
int main (int argc, char** argv){
    ros::init(argc, argv, "driver_node");
    ros::NodeHandle nh;

    std::vector<float> default_wheel_angles;
    default_wheel_angles.push_back(M_PI + 2*M_PI/3.0f);
    default_wheel_angles.push_back(M_PI);
    default_wheel_angles.push_back(M_PI - 2*M_PI/3.0f);

    nh.param("/wheelAmount", wheelAmount, 3);    
    nh.param("/wheelRads", wheelAngles, default_wheel_angles);
    nh.param("/wheelDistanceFromCenter", wheelDistanceFromCenter, 0.125f);
    nh.param("/wheelRadius", wheelRadius, 0.035f);
    nh.param("/gearboxReductionRatio", gearboxReductionRatio, 18.75f);
    nh.param("/encoderEdgesPerMotorRevolution", encoderEdgesPerMotorRevolution, 64);
    nh.param("/pidControlFrequency", pidControlFrequency, 100.0f);

    wheelSpeedToMainboardUnits = gearboxReductionRatio * encoderEdgesPerMotorRevolution / (2 * M_PI * wheelRadius * pidControlFrequency);

    //ROS_INFO_STREAM("WheelAnges " << wheelAngles[0]); //debug purpouse
    
    // Subscriber that subscribes to cmd_vel
    ros::Subscriber teleop_sub = nh.subscribe("cmd_vel", 1, teleop_callback); // alternatively "turtlebot_teleop/cmd_vel"

    // Publisher that publishes correctly formated command to serial
    ros::Publisher formated_cmd_pub = nh.advertise<std_msgs::String>("serial_write", 1);

    // The rate at which the program publishes to serial
    ros::Rate loop_rate(10);
    while(ros::ok()){
	/*
	if (format_var == 1){
	    format_cmd(cmd_vel_msg);
	    format_var = 0;
	}
	*/

        // Check for new messages in queue
        ros::spinOnce();

        //ROS_INFO_STREAM("Reading velocity.."); // debug purpouse
        std_msgs::String result;
        result.data = output_cmd;
        ROS_INFO_STREAM("Publishing: " << result); // debug purpouse
        formated_cmd_pub.publish(result);

        loop_rate.sleep();

    }
}