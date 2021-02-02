#!/bin/bash

if [ $# -ne 1 ]
then
    echo "$(date): usage $0 <catralibraries version, i.e.: 1.0.0>"

    exit
fi

version=$1


mmsStopALL.sh
sleep 2


echo "cd /opt/catramms"
cd /opt/catramms

echo "rm -f CatraLibraries"
rm -f CatraLibraries

sleep 1

linuxName=$(cat /etc/os-release | grep "^ID=" | cut -d'=' -f2)

echo "tar xvfz CatraLibraries-$version-linuxName.tar.gz"
tar xvfz CatraLibraries-$version-linuxName.tar.gz

echo "ln -s CatraLibraries-$version CatraLibraries"
ln -s CatraLibraries-$version CatraLibraries

sleep 1

cd

mmsStatusALL.sh

mmsStatusALL.sh

mmsStartALL.sh

