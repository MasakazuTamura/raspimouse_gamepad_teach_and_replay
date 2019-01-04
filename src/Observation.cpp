#include "Observation.h"
#include <iostream>
#include <string>
#include <cmath>

Observation::Observation()
{
}

/*
Observation::Observation(int left_f,int left_s, int right_s, int right_f)
{
	setValues(left_f,left_s,right_s,right_f);
}
*/

Observation::Observation(double cart_position, double cart_velocity, double pole_angle, double pole_angular)
{
	setValues(cart_position, cart_velocity, pole_angle, pole_angular);
}

/*
void Observation::setValues(int left_f,int left_s, int right_s, int right_f)
{
	lf = left_f > 0 ? left_f : 1;
	ls = left_s > 0 ? left_s : 1;
	rs = right_s > 0 ? right_s : 1;
	rf = right_f > 0 ? right_f : 1;

	log_lf = log10((double)lf);
	log_ls = log10((double)ls);
	log_rs = log10((double)rs);
	log_rf = log10((double)rf);
}
*/

void Observation::setValues(double cart_position, double cart_velocity, double pole_angle, double pole_angular)
{
	cartpos = cart_position;
	cartvel = cart_velocity;
	poleang = pole_angle;
	poleangr = pole_angular;

	log_cartpos = cartpos;
	log_cartvel = cartvel;
	log_poleang = poleang;
	log_poleangr = poleangr;
}
