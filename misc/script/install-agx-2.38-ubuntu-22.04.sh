#!/bin/sh

cd `dirname $0`
sudo dpkg -i agx-2.38.0.2-amd64-ubuntu_22.04.deb
ls -al /opt/Algoryx/AGX-2.38.0.2
sudo cp -i agx.lic  /opt/Algoryx/AGX-2.38.0.2
cd ~
echo "source /opt/Algoryx/AGX-2.38.0.2/setup_env.bash" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=/opt/Algoryx/AGX-2.38.0.2/lib:$LD_LIBRARY_PATH" >> ~/.bashrc
env | grep -i agx