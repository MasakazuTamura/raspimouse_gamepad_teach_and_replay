#!/usr/bin/env python
# Copyright 2017 Masahiro Kato
# Copyright 2017 Ryuichi Ueda
# Released under the BSD License.

import rospy, rosbag, rosparam
import math, sys, random, datetime
#from geometry_msgs.msg import Twist
#from std_srvs.srv import Trigger, TriggerResponse
#from raspimouse_ros_2.msg import LightSensorValues, ButtonValues
from raspimouse_gamepad_teach_and_replay.msg import Event
from std_msgs.msg import Int16
from pfoe_cartpole.msg import CartPoleValues, ButtonValues


class Logger():
    def __init__(self):
        #self.sensor_values = LightSensorValues()
        #self.cmd_vel = Twist()
        self.sensor_values = CartPoleValues()
        self.cmd_vel = Int16()

        self._decision = rospy.Publisher('/event',Event,queue_size=100)
        rospy.Subscriber('/buttons', ButtonValues, self.button_callback, queue_size=1)
        #rospy.Subscriber('/lightsensors', LightSensorValues, self.sensor_callback)
        #rospy.Subscriber('/cmd_vel', Twist, self.cmdvel_callback)
        rospy.Subscriber('/cartpole_state', CartPoleValues, self.sensor_callback)
        rospy.Subscriber('/key_in', Int16, self.cmdvel_callback)

        self.on = False
        self.bag_open = False

        self.done = True
        self.sensor_recieved = False
        self.cmdvel_recieved = False

    def button_callback(self,msg):
        self.on = msg.front_toggle

    def sensor_callback(self,messages):
        self.sensor_values = messages
        if not self.done:
            self.sensor_recieved = True
        self.done = self.sensor_values.done

    def cmdvel_callback(self,messages):
        self.cmd_vel = messages
        self.cmdvel_recieved = True

    def output_decision(self):
        if not self.on:
            if self.bag_open:
                self.bag.close()
                self.bag_open = False
            return
        else:
            if not self.bag_open:
                filename = datetime.datetime.today().strftime("%Y%m%d_%H%M%S") + '.bag'
                rosparam.set_param("/current_bag_file", filename)
                self.bag = rosbag.Bag(filename, 'w')
                self.bag_open = True

        if self.sensor_recieved and self.cmdvel_recieved:
            s = self.sensor_values
            a = self.cmd_vel
            e = Event()

            #e.left_side = s.left_side
            #e.right_side = s.right_side
            #e.left_forward = s.left_forward
            #e.right_forward = s.right_forward
            #e.linear_x = a.linear.x
            #e.angular_z = a.angular.z
            e.cart_position = s.cart_position
            e.cart_velocity = s.cart_velocity
            e.pole_angle = s.pole_angle
            e.pole_angular = s.pole_angular
            e.linear_x = a.data

            self.sensor_recieved = False
            self.cmdvel_recieved = False
            self._decision.publish(e)
            self.bag.write('/event', e)

    def run(self):
        rate = rospy.Rate(10)
        #data = Twist()

        while not rospy.is_shutdown():
            self.output_decision()
            rate.sleep()

if __name__ == '__main__':
    rospy.init_node('logger')
    Logger().run()
