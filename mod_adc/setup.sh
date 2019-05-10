#!/bin/sh

rm -f /dev/adc*

mknod /dev/adc0 c $1 0
chmod 666 /dev/adc0
mknod /dev/adc1 c $1 1
chmod 666 /dev/adc1
mknod /dev/adc2 c $1 2
chmod 666 /dev/adc2
