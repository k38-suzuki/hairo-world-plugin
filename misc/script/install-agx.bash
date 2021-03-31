#!/bin/bash
cd `dirname $0`
sudo dpkg -i agx-setup-2.25.0.1-x64-ubuntu_18.04-double.deb
ls -al /opt/Algoryx/AgX-2.25.0.1
sudo cp -i agx.lic  /opt/Algoryx/AgX-2.25.0.1
cd ~
TODAY=`date "+%Y%m%d"`
cp -p .profile .profile_${TODAY}
echo "source /opt/Algoryx/AgX-2.25.0.1/setup_env.bash" >> .profile
diff .profile .profile_${TODAY}
source .profile
env | grep -i agx

