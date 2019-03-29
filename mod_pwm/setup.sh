#!/bin/sh

rm -f /dev/pwm*

mknod /dev/pwm1_freq c $1 0
chmod 666 /dev/pwm1_freq

mknod /dev/pwm1_duty c $1 1
chmod 666 /dev/pwm1_duty

mknod /dev/pwm1_enabled c $1 2
chmod 666 /dev/pwm1_enabled

mknod /dev/pwm2_freq c $1 3
chmod 666 /dev/pwm2_freq

mknod /dev/pwm2_duty c $1 4
chmod 666 /dev/pwm2_duty

mknod /dev/pwm2_enabled c $1 5
chmod 666 /dev/pwm2_enabled
