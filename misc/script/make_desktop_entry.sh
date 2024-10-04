#!/bin/bash

FILE=~/Desktop/${1}.desktop
DIR=${2}

if [ ! -d $DIR ]; then
  echo $DIR 'is not exists.'
  exit
fi

if [[ -z ${1} ]]; then
  echo 'entry-name is empty.'
  exit
fi

echo -e '[Desktop Entry]\nName='${1}'\nType=Application\nIcon=org.gnome.Nautilus\nExec=gio open '$DIR >> $FILE