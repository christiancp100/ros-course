#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

ros::ServiceClient client;

// Function that calls the command robot service to drive the robot in the specified direction
void drive_bot(float lin_x, float ang_z)
{
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv))
    {
        ROS_ERROR("Failed to move the robot");
    }
}

bool is_white(int r, int g, int b)
{
    int white_pixel = 255;
    return r == white_pixel && g == white_pixel && b == white_pixel;
}

void process_image_callback(const sensor_msgs::Image img)
{
    const float lin_x = 0.2;
    const float ang_z = 0.5;
    enum Side : uint8_t
    {
        LEFT,
        CENTER,
        RIGHT,
        NOP
    } position = NOP;

    for (size_t i = 0; i < img.height * img.step; i += 3)
    {
        auto red = img.data[i];
        auto green = img.data[i + 1];
        auto blue = img.data[i + 2];

        if (is_white(red, green, blue))
        {
            auto col = i % img.step; // Calculate column directly using img.width
            if (col < img.step * 0.3)
            { // Check if column is in the first third of the image width
                position = LEFT;
            }
            else if (col > img.step * 0.7)
            { // Check if column is in the last third of the image width
                position = RIGHT;
            }
            else
            {
                position = CENTER;
            }
            break;
        }
    }

    if (position == LEFT)
    {
        drive_bot(lin_x, ang_z);
    }
    else if (position == RIGHT)
    {
        drive_bot(lin_x, -ang_z);
    }
    else if (position == CENTER)
    {
        drive_bot(lin_x, 0);
    }
    else
    {
        drive_bot(0, 0);
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Initialize client service to request services
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to the camera topic to read the image
    ros::Subscriber camera_subscriber = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    ros::spin();

    return 0;
}