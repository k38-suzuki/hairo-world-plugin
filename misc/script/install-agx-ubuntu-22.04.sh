#!/bin/sh

cd `dirname $0`
sudo dpkg -i agx-2.37.3.3-amd64-ubuntu_22.04.deb
ls -al /opt/Algoryx/AGX-2.37.3.3
sudo cp -i agx.lic  /opt/Algoryx/AGX-2.37.3.3
cd ~
echo "source /opt/Algoryx/AGX-2.37.3.3/setup_env.bash" >> .bashrc
env | grep -i agx

