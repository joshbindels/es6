#!/bin/sh

rm -f /dev/gpio

mknod /dev/gpio c 213 0
chmod 666 /dev/gpio
