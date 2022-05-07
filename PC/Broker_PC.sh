##!/bin/bash

broker="192.168.0.113"
port="1883"
topic1="test/logPC"
topic2="test/logApp"
file="Log.txt"

#--------------------------------------------------
if [ -f $file ]; then \
    rm -rf $file
fi

touch $file

mosquitto_sub -h $broker -t $topic1 -t $topic2 -v | while read $topic value; do
#mosquitto_sub -h $broker -t $topic1 -t $topic2  -p $port -v | while read topic value; do
#Add timestamp on the log
timestamp=$(date "+%d/%m/%Y %H:%M:%S")

if [ $topic == $topic1 ]; then \
		echo "$timestamp MQTT-NODO: $topic $value." >> $file
	else
		echo "$timestamp MQTT-APP: $topic $value." >> $file
fi

done


# Se corre escribiendo sh Broker_PC.sh
