#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// define a global client that can request services
ros::ServiceClient client;

// this function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("Moving robot ..");

    // request for moving robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // call the command_robot service
    if (!client.call(srv))
    {
        ROS_ERROR("Failed to call service drive_to_target");
    }
}

// this callback function continously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;

    // -1 indicates left, 0 indicates center, 1 indicates right, -2 indicates not found
    int white_pixel_position = -2;

    // loop through each pixel in the image and check if it is white
    for (int i=0; i<img.height*img.step; ++i)
    {
        bool is_white_pixel = true;
        for (int j=0; j<3; ++j)
        {
  	    is_white_pixel &= (img.data[i+j] == white_pixel);
	}
	if (is_white_pixel)
	{
  	    white_pixel_position = i%img.step < .33*img.step ? -1 : (i%img.step < .66*img.step ? 0 : 1) ;
    	    break;
	}
    }

    // If white_pixel_position is -1 rotate to left and move forward, if 0 move forward, if 1 rotate to right and move forward
    switch(white_pixel_position)
    {
        case -1:
        {
  	    drive_robot(.5, .5);
	    break;
        }
        case 0:
        {
	    drive_robot(.5, 0);
	    break;
        }
        case 1:
        {
	    drive_robot(.5, -.5);
	    break;
        }
        default:
	{
	    drive_robot(0, 0);
	    break;
	}
	  
    }      
}

int main(int argc, char** argv)
{
    // initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // subscriber to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // handle ROS communication events
    ros::spin();

    return 0;
}
