#!/bin/sh

sudo apt-get -y install \
gedit \
libompl-dev \
ompl-demos

LOGNAME=$(logname)

echo "$LOGNAME    ALL=NOPASSWD: /sbin/modprobe" >> /etc/sudoers
echo "$LOGNAME    ALL=NOPASSWD: /sbin/ip" >> /etc/sudoers
echo "$LOGNAME    ALL=NOPASSWD: /sbin/rmmod" >> /etc/sudoers
echo "$LOGNAME    ALL=NOPASSWD: /sbin/tc" >> /etc/sudoers

