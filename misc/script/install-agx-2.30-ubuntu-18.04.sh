#!/bin/sh

cd `dirname $0`
sudo dpkg -i agx-2.30.4.0-amd64-ubuntu_18.04.deb
ls -al /opt/Algoryx/AGX-2.30.4.0
sudo cp -i agx.lic  /opt/Algoryx/AGX-2.30.4.0
cd ~
TODAY=`date "+%Y%m%d"`
cp -p .profile .profile_${TODAY}
echo "source /opt/Algoryx/AGX-2.30.4.0/setup_env.bash" >> ~/.profile
diff .profile .profile_${TODAY}
source .profile
env | grep -i agx