#ifndef __OBS_H_ 
#define __OBS_H_

#include <string>
#include <vector>
#include <fstream>
#include "ros/ros.h"
using namespace std;

class Observation{
public:
	Observation();
//	Observation(int left_f,int left_s, int right_s, int right_f);
	Observation(double cart_position, double cart_velocity, double pole_angle, double pole_angular);

//	void setValues(int left_f,int left_s, int right_s, int right_f);
	void setValues(double cart_position, double cart_velocity, double pole_angle, double pole_angular);

//	int lf;
//	int ls;
//	int rs;
//	int rf;
//	double log_lf;
//	double log_ls;
//	double log_rs;
//	double log_rf;
	double cartpos;
	double cartvel;
	double poleang;
	double poleangr;
//logはraspimouseのlight sensorの為のやつ
	double log_cartpos;
	double log_cartvel;
	double log_poleang;
	double log_poleangr;
};

#endif
