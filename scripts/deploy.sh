#!/bin/bash

if [ $# -ne 1 ]
then
    echo "$(date): usage $0 <catralibraries version, i.e.: 1.0.0>"

    exit
fi

version=$1

removePreviousVersions()
{
	currentPathNameVersion=$(readlink -f /opt/catramms/CatraLibraries)
	if [ "${currentPathNameVersion}" != "" ];
	then
		tenDaysInMinutes=14400

		echo "Remove previous versions (retention $tenDaysInMinutes)"
		echo "find /opt/catramms -maxdepth 1 -mmin +$tenDaysInMinutes -name \"CatraLibraries-*\" -not -path \"${currentPathNameVersion}*\" -exec rm -rf {} \;"
		find /opt/catramms -maxdepth 1 -mmin +$tenDaysInMinutes -name "CatraLibraries-*" -not -path "${currentPathNameVersion}*" -exec rm -rf {} \;
	fi
}

if [ ! -f "/opt/catramms/CatraLibraries-$version.tar.gz" ]; then
    echo "/opt/catramms/CatraLibraries-$version.tar.gz does not exist."

	exit
fi

#sleepIfNeeded
removePreviousVersions

mmsStopALL.sh

echo "cd /opt/catramms"
cd /opt/catramms

echo "rm -f CatraLibraries"
rm -f CatraLibraries

sleep 1

#linuxName=$(cat /etc/os-release | grep "^ID=" | cut -d'=' -f2)

#echo "tar xvfz CatraLibraries-$version-$linuxName.tar.gz"
#tar xvfz CatraLibraries-$version-$linuxName.tar.gz
echo "tar xfz CatraLibraries-$version.tar.gz"
tar xfz CatraLibraries-$version.tar.gz

echo "ln -s CatraLibraries-$version CatraLibraries"
ln -s CatraLibraries-$version CatraLibraries

cd

mmsStatusALL.sh

echo ""

mmsStatusALL.sh

mmsStartALL.sh

