#!/bin/bash


if [ $# -ne 2 ]
then
    echo "usage $0 <version tag, i.e.: 1.0.0> <tag message>"
	echo "Reminder to list tags: git tag -n --sort=taggerdate"

    exit
fi

tagName=$1
tagMessage=$2


echo "git commit -am $tagMessage"
git commit -am "$tagMessage"

echo "./prepareToDeploy/setTag.sh $tagName $tagMessage"
./prepareToDeploy/setTag.sh $tagName "$tagMessage"

echo "git commit -am $tagMessage"
git commit -am "$tagMessage"

echo "git push"
git push

