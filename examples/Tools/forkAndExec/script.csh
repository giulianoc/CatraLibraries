#! /bin/sh

echo "param 0: $0" > /tmp/aaa.txt
echo "param 1: $1" >> /tmp/aaa.txt
echo "param 2: $2" >> /tmp/aaa.txt
echo "param 3: $3" >> /tmp/aaa.txt

sleep 15

exit 2

