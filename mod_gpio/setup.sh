#!/bin/sh

rm -f /dev/gpio

mknod /dev/gpio c $1 0
chmod 666 /dev/gpio
