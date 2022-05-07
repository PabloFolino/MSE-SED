##!/bin/bash


# topic1="control/logEsp"
#topic2="control/logApp"

broker="192.168.0.113"
port="1883"
topic1="control/logEsp"
topic2="control/logApp"
file="Log.txt"

#--------------------------------------------------
if [ -f $file ]; then \
    rm -rf $file
fi

touch $file

mosquitto_sub -t $topic1 -t $topic2 -h $broker -p $port -v | while read topic value
do
#Add timestamp on the log
timestamp=$(date "+%d/%m/%Y %H:%M:%S")

if [ $topic == $topic1 ]
 then 
	if [ "$value" == "Reset" ]
	 then 
		echo "$timestamp MQTT-APP: $topic $value." >> $file
	 else
		echo "$timestamp MQTT-NODO: $topic $value." >> $file
	fi
 else
    echo "$timestamp MQTT-APP: $topic $value." >> $file
fi

done
