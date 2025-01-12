#include <stdio.h>
#include <string>
#include <algorithm>
#include <cmath>
#include <signal.h>
#include <termios.h>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_listener.h>
#include <sensor_msgs/JointState.h>
#include <geometry_msgs/PoseStamped.h>
#include <trajectory_msgs/JointTrajectory.h>
#include <trajectory_msgs/JointTrajectoryPoint.h>

class SIGVerseFetchTeleopKey
{
private:

  static const char KEYCODE_1 = 0x31;
  static const char KEYCODE_2 = 0x32;
  static const char KEYCODE_3 = 0x33;
  static const char KEYCODE_4 = 0x34;
  static const char KEYCODE_5 = 0x35;
  static const char KEYCODE_6 = 0x36;
  static const char KEYCODE_7 = 0x37;

  static const char KEYCODE_UP    = 0x41;
  static const char KEYCODE_DOWN  = 0x42;
  static const char KEYCODE_RIGHT = 0x43;
  static const char KEYCODE_LEFT  = 0x44;

  static const char KEYCODE_A = 0x61;
  static const char KEYCODE_C = 0x63;
  static const char KEYCODE_D = 0x64;
  static const char KEYCODE_E = 0x65;
  static const char KEYCODE_F = 0x66;
  static const char KEYCODE_G = 0x67;
  static const char KEYCODE_H = 0x68;
  static const char KEYCODE_I = 0x69;
  static const char KEYCODE_J = 0x6a;
  static const char KEYCODE_K = 0x6b;
  static const char KEYCODE_L = 0x6c;
  static const char KEYCODE_M = 0x6d;
  static const char KEYCODE_N = 0x6e;
  static const char KEYCODE_O = 0x6f;
  static const char KEYCODE_Q = 0x71;
  static const char KEYCODE_R = 0x72;
  static const char KEYCODE_S = 0x73;
  static const char KEYCODE_U = 0x75;
  static const char KEYCODE_W = 0x77;
  static const char KEYCODE_X = 0x78;
  static const char KEYCODE_Y = 0x79;
  static const char KEYCODE_Z = 0x7a;

  static const char KEYCODE_COMMA  = 0x2c;
  static const char KEYCODE_PERIOD = 0x2e;
  static const char KEYCODE_SPACE  = 0x20;

  std::map<std::string, double> arm_joint_state_map_;
  std::map<std::string, double> arm_joint_max_map_;
  std::map<std::string, double> arm_joint_min_map_;

public:
  SIGVerseFetchTeleopKey();

  static void rosSigintHandler(int sig);
  static int  canReceive(int fd);

//  void messageCallback(const std_msgs::String::ConstPtr& message);
  void jointStateCallback(const sensor_msgs::JointState::ConstPtr& joint_state);
//  void sendMessage(const std::string &message);
  void moveBaseTwist(double linear_x, double linear_y, double angular_z);
  void operateTorso(const double torso_lift_pos, const double duration_sec);
  void operateHead(const double head_1_pos, const double head_2_pos, const double duration_sec);
  void operateArm(const std::vector<double> & positions, const double duration_sec);
  void operateArm(const int joint_number, const double arm_pos, const double duration_sec);
//  double getDurationRot(const double next_pos, const double current_pos);
  void operateHand(bool grasp);

  void showHelp();
  void showHelpArm(const std::string &arm_name);
  int run();

private:
  // Last position and previous position of torso_lift_joint
  double torso_lift_joint_pos1_;
  double torso_lift_joint_pos2_;

  ros::NodeHandle node_handle_;

  ros::Subscriber sub_joint_state_;
  ros::Publisher  pub_base_twist_;
  ros::Publisher  pub_torso_trajectory_;
  ros::Publisher  pub_head_trajectory_;
  ros::Publisher  pub_arm_trajectory_;
  ros::Publisher  pub_gripper_trajectory_;

  tf::TransformListener listener_;
};


SIGVerseFetchTeleopKey::SIGVerseFetchTeleopKey()
{
  torso_lift_joint_pos1_ = 0.0;
  torso_lift_joint_pos2_ = 0.0;

  arm_joint_state_map_["shoulder_pan_joint"]  = 0.0;
  arm_joint_state_map_["shoulder_lift_joint"] = 0.0;
  arm_joint_state_map_["upperarm_roll_joint"] = 0.0;
  arm_joint_state_map_["elbow_flex_joint"]    = 0.0;
  arm_joint_state_map_["forearm_roll_joint"]  = 0.0;
  arm_joint_state_map_["wrist_flex_joint"]    = 0.0;
  arm_joint_state_map_["wrist_roll_joint"]    = 0.0;

  arm_joint_max_map_["shoulder_pan_joint"]  = +1.605;
  arm_joint_max_map_["shoulder_lift_joint"] = +1.518;
  arm_joint_max_map_["upperarm_roll_joint"] = +9999.9;
  arm_joint_max_map_["elbow_flex_joint"]    = +2.251;
  arm_joint_max_map_["forearm_roll_joint"]  = +9999.9;
  arm_joint_max_map_["wrist_flex_joint"]    = +2.181f;
  arm_joint_max_map_["wrist_roll_joint"]    = +9999.9;

  arm_joint_min_map_["shoulder_pan_joint"]  = -1.605;
  arm_joint_min_map_["shoulder_lift_joint"] = -1.221;
  arm_joint_min_map_["upperarm_roll_joint"] = -9999.9;
  arm_joint_min_map_["elbow_flex_joint"]    = -2.251;
  arm_joint_min_map_["forearm_roll_joint"]  = -9999.9;
  arm_joint_min_map_["wrist_flex_joint"]    = -2.181f;
  arm_joint_min_map_["wrist_roll_joint"]    = -9999.9;
}


void SIGVerseFetchTeleopKey::rosSigintHandler(int sig)
{
  ros::shutdown();
}


int SIGVerseFetchTeleopKey::canReceive( int fd )
{
  fd_set fdset;
  int ret;
  struct timeval timeout;
  FD_ZERO( &fdset );
  FD_SET( fd , &fdset );

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  return select( fd+1 , &fdset , NULL , NULL , &timeout );
}

//void SIGVerseFetchTeleopKey::messageCallback(const std_msgs::String::ConstPtr& message)
//{
//  ROS_INFO("Subscribe message: %s", message->data.c_str());
//}

void SIGVerseFetchTeleopKey::jointStateCallback(const sensor_msgs::JointState::ConstPtr& joint_state)
{
  for(int i=0; i<joint_state->name.size(); i++)
  {
//    if(joint_state->name[i] == "gripper_left_finger_joint")
//    {
//      puts(("gripper_left_finger_joint="+std::to_string(joint_state->position[i])).c_str());
//    }

    if(joint_state->name[i] == "torso_lift_joint")
    {
      torso_lift_joint_pos2_ = torso_lift_joint_pos1_;
      torso_lift_joint_pos1_ = joint_state->position[i];
    }
    if(joint_state->name[i] == "shoulder_pan_joint") { arm_joint_state_map_["shoulder_pan_joint"]  = joint_state->position[i]; }
    if(joint_state->name[i] == "shoulder_lift_joint"){ arm_joint_state_map_["shoulder_lift_joint"] = joint_state->position[i]; }
    if(joint_state->name[i] == "upperarm_roll_joint"){ arm_joint_state_map_["upperarm_roll_joint"] = joint_state->position[i]; }
    if(joint_state->name[i] == "elbow_flex_joint")   { arm_joint_state_map_["elbow_flex_joint"]    = joint_state->position[i]; }
    if(joint_state->name[i] == "forearm_roll_joint") { arm_joint_state_map_["forearm_roll_joint"]  = joint_state->position[i]; }
    if(joint_state->name[i] == "wrist_flex_joint")   { arm_joint_state_map_["wrist_flex_joint"]    = joint_state->position[i]; }
    if(joint_state->name[i] == "wrist_roll_joint")   { arm_joint_state_map_["wrist_roll_joint"]    = joint_state->position[i]; }
  }
}

//void SIGVerseFetchTeleopKey::sendMessage(const std::string &message)
//{
//  ROS_INFO("Send message:%s", message.c_str());
//
//  std_msgs::String string_msg;
//  string_msg.data = message;
//  pub_msg_.publish(string_msg);
//}

void SIGVerseFetchTeleopKey::moveBaseTwist(double linear_x, double linear_y, double angular_z)
{
  geometry_msgs::Twist twist;

  twist.linear.x  = linear_x;
  twist.linear.y  = linear_y;
  twist.angular.z = angular_z;
  pub_base_twist_.publish(twist);
}

void SIGVerseFetchTeleopKey::operateTorso(const double torso_lift_pos, const double duration_sec)
{
  trajectory_msgs::JointTrajectory joint_trajectory;
  joint_trajectory.joint_names.push_back("torso_lift_joint");

  trajectory_msgs::JointTrajectoryPoint torso_joint_point;

  torso_joint_point.positions = { torso_lift_pos };

  torso_joint_point.time_from_start = ros::Duration(duration_sec);
  joint_trajectory.points.push_back(torso_joint_point);
  pub_torso_trajectory_.publish(joint_trajectory);
}

void SIGVerseFetchTeleopKey::operateHead(const double head_1_pos, const double head_2_pos, const double duration_sec)
{
  trajectory_msgs::JointTrajectory joint_trajectory;
  joint_trajectory.joint_names.push_back("head_pan_joint");
  joint_trajectory.joint_names.push_back("head_tilt_joint");

  trajectory_msgs::JointTrajectoryPoint head_joint_point;

  head_joint_point.positions = { head_1_pos, head_2_pos };

  head_joint_point.time_from_start = ros::Duration(duration_sec);
  joint_trajectory.points.push_back(head_joint_point);
  pub_torso_trajectory_.publish(joint_trajectory);
}

void SIGVerseFetchTeleopKey::operateArm(const std::vector<double> & positions, const double duration_sec)
{
  trajectory_msgs::JointTrajectory joint_trajectory;
  joint_trajectory.joint_names.push_back("shoulder_pan_joint");
  joint_trajectory.joint_names.push_back("shoulder_lift_joint");
  joint_trajectory.joint_names.push_back("upperarm_roll_joint");
  joint_trajectory.joint_names.push_back("elbow_flex_joint");
  joint_trajectory.joint_names.push_back("forearm_roll_joint");
  joint_trajectory.joint_names.push_back("wrist_flex_joint");
  joint_trajectory.joint_names.push_back("wrist_roll_joint");
  
  trajectory_msgs::JointTrajectoryPoint arm_joint_point;

  arm_joint_point.positions = positions;

  arm_joint_point.time_from_start = ros::Duration(duration_sec);
  joint_trajectory.points.push_back(arm_joint_point);
  pub_arm_trajectory_.publish(joint_trajectory);
}

void SIGVerseFetchTeleopKey::operateArm(const int joint_number, const double add_pos, const double duration_sec)
{
  std::vector<double> positions =
  {
    arm_joint_state_map_["shoulder_pan_joint"],  arm_joint_state_map_["shoulder_lift_joint"], 
    arm_joint_state_map_["upperarm_roll_joint"], arm_joint_state_map_["elbow_flex_joint"], arm_joint_state_map_["forearm_roll_joint"], 
    arm_joint_state_map_["wrist_flex_joint"],    arm_joint_state_map_["wrist_roll_joint"]
  };

  std::string joint_name = "";

  switch(joint_number)
  {
    case 1:{ joint_name = "shoulder_pan_joint";  break; }
    case 2:{ joint_name = "shoulder_lift_joint"; break; }
    case 3:{ joint_name = "upperarm_roll_joint"; break; }
    case 4:{ joint_name = "elbow_flex_joint";    break; }
    case 5:{ joint_name = "forearm_roll_joint";  break; }
    case 6:{ joint_name = "wrist_flex_joint";    break; }
    case 7:{ joint_name = "wrist_roll_joint";    break; }
  }
  
  // Clamp
  double clamped_next_pos = std::min(std::max(arm_joint_min_map_[joint_name], arm_joint_state_map_[joint_name]+add_pos), arm_joint_max_map_[joint_name]);

  positions[joint_number-1] = clamped_next_pos;

  this->operateArm(positions, duration_sec);
}

//double SIGVerseFetchTeleopKey::getDurationRot(const double next_pos, const double current_pos)
//{
//  return std::max<double>((std::abs(next_pos - current_pos) * 1.2), 1.0);
//}

void SIGVerseFetchTeleopKey::operateHand(bool is_hand_open)
{
  trajectory_msgs::JointTrajectory joint_trajectory;
  joint_trajectory.joint_names.push_back("l_gripper_finger_joint");
  joint_trajectory.joint_names.push_back("r_gripper_finger_joint");

  trajectory_msgs::JointTrajectoryPoint gripper_joint_point;

  if(is_hand_open)
  {
    ROS_DEBUG("Grasp");
    gripper_joint_point.positions = { 0.0, 0.0 };
  }
  else
  {
    ROS_DEBUG("Open hand");
    gripper_joint_point.positions = { 0.05, 0.05 };
  }

  gripper_joint_point.time_from_start = ros::Duration(2);
  joint_trajectory.points.push_back(gripper_joint_point);
  pub_gripper_trajectory_.publish(joint_trajectory);
}


void SIGVerseFetchTeleopKey::showHelp()
{
  puts("Operate by Keyboard");
  puts("---------------------------");
  puts("arrow keys : Move Fetch");
  puts("space      : Stop Fetch");
  puts("r/f : Increase/Decrease Moving Speed");
  puts("---------------------------");
  puts("y : Rotate Arm - Upward");
  puts("h : Rotate Arm - Horizontal");
  puts("n : Rotate Arm - Downward");
  puts("---------------------------");
  puts("q/a/z : Up/Stop/Down Torso");
  puts("---------------------------");
  puts("w/s/x : Turn Head Left/Front/Right ");
  puts("e/d/c : Turn Head Up/Front/Down");
  puts("---------------------------");
  puts("u/j/m : Control shoulder_pan/shoulder_lift/upperarm_roll ");
  puts("i/k   : Control elbow_flex/forearm_roll ");
  puts("o/l   : Control wrist_flex/wrist_roll ");
  puts("---------------------------");
  puts("g : Grasp/Open Hand");
}

void SIGVerseFetchTeleopKey::showHelpArm(const std::string &arm_name)
{
  puts("");
  puts("---------------------------");
  puts(("Control "+arm_name).c_str());
  puts("u/j/m : + / Stop / - ");
  puts("---------------------------");
  puts("q : Quit");
  puts("");
}


int SIGVerseFetchTeleopKey::run()
{
  char c;
  int  ret;
  char buf[1024];

  /////////////////////////////////////////////
  // get the console in raw mode
  int kfd = 0;
  struct termios cooked;

  struct termios raw;
  tcgetattr(kfd, &cooked);
  memcpy(&raw, &cooked, sizeof(struct termios));
  raw.c_lflag &=~ (ICANON | ECHO);
  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  tcsetattr(kfd, TCSANOW, &raw);
  /////////////////////////////////////////////

  showHelp();

  // Override the default ros sigint handler.
  // This must be set after the first NodeHandle is created.
  signal(SIGINT, rosSigintHandler);

  ros::Rate loop_rate(40);

  std::string sub_joint_state_topic_name;
  std::string pub_base_twist_topic_name;
  std::string pub_torso_trajectory_topic_name;
  std::string pub_head_trajectory_topic_name;
  std::string pub_arm_trajectory_topic_name;
  std::string pub_gripper_trajectory_topic_name;

  node_handle_.param<std::string>("fetch_teleop_key/sub_joint_state_topic_name",        sub_joint_state_topic_name,        "/joint_states");
  node_handle_.param<std::string>("fetch_teleop_key/pub_base_twist_topic_name",         pub_base_twist_topic_name,         "/base_controller/command");
  node_handle_.param<std::string>("fetch_teleop_key/pub_torso_trajectory_topic_name",   pub_torso_trajectory_topic_name,   "/torso_controller/command");
  node_handle_.param<std::string>("fetch_teleop_key/pub_head_trajectory_topic_name",    pub_head_trajectory_topic_name,    "/head_controller/command");
  node_handle_.param<std::string>("fetch_teleop_key/pub_arm_trajectory_topic_name",     pub_arm_trajectory_topic_name,     "/arm_controller/command");
  node_handle_.param<std::string>("fetch_teleop_key/pub_gripper_trajectory_topic_name", pub_gripper_trajectory_topic_name, "/gripper_controller/command");

  sub_joint_state_        = node_handle_.subscribe<sensor_msgs::JointState>(sub_joint_state_topic_name, 10, &SIGVerseFetchTeleopKey::jointStateCallback, this);
  pub_base_twist_         = node_handle_.advertise<geometry_msgs::Twist>(pub_base_twist_topic_name, 10);
  pub_torso_trajectory_   = node_handle_.advertise<trajectory_msgs::JointTrajectory>(pub_torso_trajectory_topic_name, 10);
  pub_head_trajectory_    = node_handle_.advertise<trajectory_msgs::JointTrajectory>(pub_head_trajectory_topic_name, 10);
  pub_arm_trajectory_     = node_handle_.advertise<trajectory_msgs::JointTrajectory>(pub_arm_trajectory_topic_name, 10);
  pub_gripper_trajectory_ = node_handle_.advertise<trajectory_msgs::JointTrajectory>(pub_gripper_trajectory_topic_name, 10);

  const float linear_coef  = 0.3f;
  const float angular_coef = 0.5f;

  float move_speed = 1.0f;
  bool is_hand_open = false;

  while (ros::ok())
  {
    if(canReceive(kfd))
    {
      // get the next event from the keyboard
      if((ret = read(kfd, &buf, sizeof(buf))) < 0)
      {
        perror("read():");
        exit(EXIT_FAILURE);
      }

      c = buf[ret-1];
          
      switch(c)
      {
        case KEYCODE_UP:
        {
          ROS_DEBUG("Go Forward");
          moveBaseTwist(+linear_coef*move_speed, 0.0, 0.0);
          break;
        }
        case KEYCODE_DOWN:
        {
          ROS_DEBUG("Go Backward");
          moveBaseTwist(-linear_coef*move_speed, 0.0, 0.0);
          break;
        }
        case KEYCODE_RIGHT:
        {
          ROS_DEBUG("Go Right");
          moveBaseTwist(0.0, 0.0, -angular_coef*move_speed);
          break;
        }
        case KEYCODE_LEFT:
        {
          ROS_DEBUG("Go Left");
          moveBaseTwist(0.0, 0.0, +angular_coef*move_speed);
          break;
        }
        case KEYCODE_SPACE:
        {
          ROS_DEBUG("Stop");
          moveBaseTwist(0.0, 0.0, 0.0);
          break;
        }
        case KEYCODE_R:
        {
          ROS_DEBUG("Move Speed Up");
          move_speed *= 2;
          if(move_speed > 2  ){ move_speed=2; }
          break;
        }
        case KEYCODE_F:
        {
          ROS_DEBUG("Move Speed Down");
          move_speed /= 2;
          if(move_speed < 0.125){ move_speed=0.125; }
          break;
        }
        case KEYCODE_Y:
        {
          ROS_DEBUG("Rotate Arm - Upward");
          operateArm({ 0.0, -0.8, 0.0, +0.4, 0.0, +0.4, 0.0 }, 3.0);
          break;
        }
        case KEYCODE_H:
        {
          ROS_DEBUG("Rotate Arm - Horizontal");
          operateArm({ 0.0, +1.0, 0.0, -2.25, 0.0, +1.25, 0.0 }, 3.0);
          break;
        }
        case KEYCODE_N:
        {
          ROS_DEBUG("Rotate Arm - Downward");
          operateArm({ 0.0, +1.0, 0.0, 0.0, 0.0, -1.0, 0.0 }, 3.0);
          break;
        }
        case KEYCODE_Q:
        {
          ROS_DEBUG("Torso Height - Up");
          operateTorso(0.4, std::max<int>((int)(std::abs(0.4 - torso_lift_joint_pos1_) / 0.05), 1));
          break;
        }
        case KEYCODE_A:
        {
          ROS_DEBUG("Torso Height - Stop");
          operateTorso(2.0*torso_lift_joint_pos1_-torso_lift_joint_pos2_, 0.5);
          break;
        }
        case KEYCODE_Z:
        {
          ROS_DEBUG("Torso Height - Down");
          operateTorso(0.0, std::max<int>((int)(std::abs(0.0 - torso_lift_joint_pos1_) / 0.05), 1));
          break;
        }
        case KEYCODE_W:
        {
          ROS_DEBUG("Turn Head - Left");
          operateHead(+1.57, 0.0, 2.0);
          break;
        }
        case KEYCODE_S:
        {
          ROS_DEBUG("Turn Head - Front");
          operateHead(0.0, 0.0, 2.0);
          break;
        }
        case KEYCODE_X:
        {
          ROS_DEBUG("Turn Head - Right");
          operateHead(-1.57, 0.0, 2.0);
          break;
        }
        case KEYCODE_E:
        {
          ROS_DEBUG("Turn Head - Up");
          operateHead(0.0, -0.785, 2.0);
          break;
        }
        case KEYCODE_D:
        {
          ROS_DEBUG("Turn Head - Front");
          operateHead(0.0, 0.0, 2.0);
          break;
        }
        case KEYCODE_C:
        {
          ROS_DEBUG("Turn Head - Down");
          operateHead(0.0, +1.57, 2.0);
          break;
        }
        case KEYCODE_U:
        case KEYCODE_J:
        case KEYCODE_M:
        case KEYCODE_I:
        case KEYCODE_K:
        case KEYCODE_O:
        case KEYCODE_L:
        {
          int joint_number;

          switch(c)
          {
            case KEYCODE_U:{ joint_number = 1; ROS_DEBUG("Control shoulder_pan_joint");  showHelpArm("shoulder_pan_joint"); break; }
            case KEYCODE_J:{ joint_number = 2; ROS_DEBUG("Control shoulder_lift_joint"); showHelpArm("shoulder_lift_joint"); break; }
            case KEYCODE_M:{ joint_number = 3; ROS_DEBUG("Control upperarm_roll_joint"); showHelpArm("upperarm_roll_joint"); break; }
            case KEYCODE_I:{ joint_number = 4; ROS_DEBUG("Control elbow_flex_joint");    showHelpArm("elbow_flex_joint"); break; }
            case KEYCODE_K:{ joint_number = 5; ROS_DEBUG("Control forearm_roll_joint");  showHelpArm("forearm_roll_joint"); break; }
            case KEYCODE_O:{ joint_number = 6; ROS_DEBUG("Control wrist_flex_joint");    showHelpArm("wrist_flex_joint"); break; }
            case KEYCODE_L:{ joint_number = 7; ROS_DEBUG("Control wrist_roll_joint");    showHelpArm("wrist_roll_joint"); break; }
          }

          bool is_in_control = true;

          while(is_in_control)
          {
            ros::spinOnce();
            loop_rate.sleep();

            if(!canReceive(kfd)){ continue; }

            if(read(kfd, &c, 1) < 0)
            {
              perror("read():");
              exit(EXIT_FAILURE);
            }

            switch(c)
            {
              case KEYCODE_U:{ ROS_DEBUG("Arm  +  "); operateArm(joint_number, +1.0, 2.0); break; }
              case KEYCODE_J:{ ROS_DEBUG("Arm Stop"); operateArm(joint_number,  0.0, 2.0); break; }
              case KEYCODE_M:{ ROS_DEBUG("Arm  -  "); operateArm(joint_number, -1.0, 2.0); break; }
              case KEYCODE_Q:
              {
                is_in_control = false;
                showHelp();
                break;
              }
            }
          }
          break;
        }
        case KEYCODE_G:
        {
          operateHand(is_hand_open);

          is_hand_open = !is_hand_open;
          break;
        }
      }
    }

    ros::spinOnce();

    loop_rate.sleep();
  }

  /////////////////////////////////////////////
  // cooked mode
  tcsetattr(kfd, TCSANOW, &cooked);
  /////////////////////////////////////////////

  return EXIT_SUCCESS;
}


int main(int argc, char** argv)
{
  ros::init(argc, argv, "fetch_teleop_key");
  SIGVerseFetchTeleopKey fetch_teleop_key;
  return fetch_teleop_key.run();
}
