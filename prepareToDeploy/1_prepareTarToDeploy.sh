#!/bin/bash


if [ $# -ne 2 ]
then
    echo "usage $0 <version tag, i.e.: 1.0.0> <tag message>"
	echo "Reminder to list tags: git tag -n --sort=taggerdate"

    exit
fi

tagName=$1
tagMessage=$2

RED='\033[0;31m'

echo ""
printf "${RED}git commit -am $tagMessage\n"
git commit -am "$tagMessage"

echo ""
printf "${RED}./prepareToDeploy/setTag.sh $tagName $tagMessage"
./prepareToDeploy/setTag.sh $tagName "$tagMessage"

echo ""
printf "${RED}git commit -am $tagMessage"
git commit -am "$tagMessage"

echo ""
printf "${RED}git push"
git push

read -n 1 -s -r -p "Press any key to continue"

echo ""
printf "${RED}./prepareToDeploy/createTarVersion.sh"
./prepareToDeploy/createTarVersion.sh

