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
NC='\033[0m' # No Color


echo ""
printf "${RED}git commit -am $tagMessage\n"
printf "${NC}"
git commit -am "$tagMessage"

echo ""
printf "${RED}./prepareToDeploy/setTag.sh $tagName $tagMessage"
printf "${NC}"
./prepareToDeploy/setTag.sh $tagName "$tagMessage"

echo ""
printf "${RED}git commit -am $tagMessage"
printf "${NC}"
git commit -am "$tagMessage"

echo ""
printf "${RED}git push"
printf "${NC}"
git push

read -n 1 -s -r -p "Press any key to continue"

echo ""
printf "${RED}./prepareToDeploy/createTarVersion.sh"
printf "${NC}"
./prepareToDeploy/createTarVersion.sh

