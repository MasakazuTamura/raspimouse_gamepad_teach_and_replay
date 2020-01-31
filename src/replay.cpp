#include "ros/ros.h"
#include "Event.h"
#include "Episodes.h"
#include "ParticleFilter.h"
#include <iostream>
#include <fstream>
#include <signal.h>
#include <chrono>
#include <thread>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <ros/package.h> 
//#include "std_srvs/Trigger.h"
//#include "geometry_msgs/Twist.h"
//#include "raspimouse_ros_2/LightSensorValues.h"
//#include "raspimouse_ros_2/TimedMotion.h"
//#include "raspimouse_ros_2/ButtonValues.h"
#include "std_msgs/Int16.h"
#include "pfoe_cartpole/CartPoleValues.h"
#include "pfoe_cartpole/ButtonValues.h"
#include "raspimouse_gamepad_teach_and_replay/Event.h"
#include "ParticleFilter.h"
#include "raspimouse_gamepad_teach_and_replay/PFoEOutput.h"
using namespace ros;

Episodes ep;
ParticleFilter pf(1000,&ep);

Observation sensor_values;

NodeHandle *np;
//int sum_forward = 0;
bool cartpole_done = false;

bool on = false;
bool bag_read = false;
bool sensor_recieved = false;

//void buttonCallback(const raspimouse_ros_2::ButtonValues::ConstPtr& msg)
void buttonCallback(const pfoe_cartpole::ButtonValues::ConstPtr& msg)
{
	on = msg->mid_toggle;
}

/*
void sensorCallback(const raspimouse_ros_2::LightSensorValues::ConstPtr& msg)
{
	sensor_values.setValues(msg->left_forward,msg->left_side,msg->right_side,msg->right_forward);
	sum_forward = msg->sum_forward;
}
*/
void sensorCallback(const pfoe_cartpole::CartPoleValues::ConstPtr& msg)
{
	sensor_values.setValues(msg->cart_position, msg->cart_velocity, msg->pole_angle, msg->pole_angular);
	cartpole_done = msg->done;
	sensor_recieved = true;
}

/*
void on_shutdown(int sig)
{
//	ros::ServiceClient motor_off = np->serviceClient<std_srvs::Trigger>("motor_off");
//	std_srvs::Trigger t;
//	motor_off.call(t);

	shutdown();
}
*/

void readEpisodes(string file)
{
	ep.reset();

	rosbag::Bag bag1(file, rosbag::bagmode::Read);

	vector<std::string> topics;
	topics.push_back("/event");
	
	rosbag::View view(bag1, rosbag::TopicQuery(topics));

//	double start = view.getBeginTime().toSec() + 5.0; //discard first 5 sec
//	double end = view.getEndTime().toSec() - 5.0; //discard last 5 sec
	double start = view.getBeginTime().toSec();
	double end = view.getEndTime().toSec();
	for(auto i : view){
		auto s = i.instantiate<raspimouse_gamepad_teach_and_replay::Event>();

//		Observation obs(s->left_forward,s->left_side,s->right_side,s->right_forward);
		Observation obs(s->cart_position, s->cart_velocity, s->pole_angle, s->pole_angular);
//		Action a = {s->linear_x,s->angular_z};
		Action a = {s->linear_x};
		Event e(obs,a,0.0);
		e.time = i.getTime();

		if(e.time.toSec() < start)
			continue;

		ep.append(e);

		if(e.time.toSec() > end)
			break;
	}
}

int main(int argc, char **argv)
{
	init(argc,argv,"go_around");
	NodeHandle n;
	np = &n;

//	Subscriber sub = n.subscribe("lightsensors", 1, sensorCallback);
	Subscriber sub = n.subscribe("cartpole_state", 1, sensorCallback);
	Subscriber sub_b = n.subscribe("buttons", 1, buttonCallback);
//	Publisher cmdvel = n.advertise<geometry_msgs::Twist>("cmd_vel", 1);
	Publisher cmdvel = n.advertise<std_msgs::Int16>("key_in", 1);
	Publisher pfoe_out = n.advertise<raspimouse_gamepad_teach_and_replay::PFoEOutput>("pfoe_out", 100);
//	ros::ServiceClient motor_on = n.serviceClient<std_srvs::Trigger>("motor_on");
//	ros::ServiceClient tm = n.serviceClient<raspimouse_ros_2::TimedMotion>("timed_motion");

//	signal(SIGINT, on_shutdown);

//	motor_on.waitForExistence();
//	std_srvs::Trigger t;
//	motor_on.call(t);

//	geometry_msgs::Twist msg;
	std_msgs::Int16 msg;
//	pf.init();	//it cause segmentation fault on desctop
//	Rate loop_rate(10);
	Rate loop_rate(20);
//	Action act = {0.0,0.0};
	Action act = {0};
	std::vector<double> elapsedtimes;
		raspimouse_gamepad_teach_and_replay::PFoEOutput out;
	while(ok()){
		if(not on){
//			cout << "idle" << endl;
			bag_read = false;
			spinOnce();
			loop_rate.sleep();
			continue;
		}else if(not bag_read){
			string bagfile;
			n.getParam("/current_bag_file", bagfile);
			readEpisodes(bagfile);
			bag_read = true;
			pf.init();
			spinOnce();
			loop_rate.sleep();
			continue;
		}
		if(not sensor_recieved){
			spinOnce();
			loop_rate.sleep();
			continue;
		}

		if(cartpole_done){
//			cout << "--------------" << endl;
			double sum = 0.0;
			for(auto elem : elapsedtimes){
//			    std::cout<< elem << endl;
				sum += elem;
			}
			sum /= elapsedtimes.size();
			cout << "\r**************" << endl;
			cout << "\rroop time =" << sum << "micro sec." << endl;
			cout << "\r**************" << endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			elapsedtimes.clear();
		}

		std::chrono::system_clock::time_point  start, end;
		start = std::chrono::system_clock::now();

		act = pf.sensorUpdate(&sensor_values, &act, &ep, &out);
//		msg.linear.x = act.linear_x;
		msg.data = act.linear_x;
		out.linear_x = act.linear_x;
//		msg.angular.z = act.angular_z;
//		out.angular_z = act.angular_z;

//		out.left_forward = sensor_values.lf;
//		out.left_side = sensor_values.ls;
//		out.right_forward = sensor_values.rf;
//		out.right_side = sensor_values.rs;
		out.cart_position = sensor_values.cartpos;
		out.cart_velocity = sensor_values.cartvel;
		out.pole_angle = sensor_values.poleang;
		out.pole_angular = sensor_values.poleangr;

		cmdvel.publish(msg);
		pfoe_out.publish(out);
		pf.motionUpdate(&ep);

		end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
		elapsedtimes.push_back(elapsed);

		sensor_recieved = false;
		spinOnce();
		loop_rate.sleep();
	}
	return 0;
}
