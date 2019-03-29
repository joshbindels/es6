#!/bin/sh

rm -f /dev/pwm_freq /dev/pwm_duty /dev/pwm_enabled

mknod /dev/pwm_freq c $1 0
chmod 666 /dev/pwm_freq

mknod /dev/pwm_duty c $1 1
chmod 666 /dev/pwm_duty

mknod /dev/pwm_enabled c $1 2
chmod 666 /dev/pwm_enabled
