#!/bin/bash

# Release: V1R5 - 08/02/2018 - Joshua Crawford

logfile=/logs/web-app/public/log/QRFLlog_Network.txt
settingsfile=/home/debian/QRFL_Network_Settings.json

## Check settings file exists
if [ -e "$settingsfile" ]
then
	echo "........................................." > "$logfile"
	echo ".......... System Network Check ........." >> "$logfile"
	echo "........................................." >> "$logfile"

	## Time stamp
	Test_Time="$(date -R)"
	echo "Network Test Start Time: $Test_Time"
	echo "Network Test Start Time: $Test_Time" > "$logfile"

	## Get gateway setting from the system
	Gateway_IP="$(cat $settingsfile | jq -r '.Gateway')"
	echo -e "Gateway IP:$Gateway_IP"
	echo -e "Gateway IP:$Gateway_IP" >> "$logfile"
	retry=8
	declare -i fails=0

	## Loop 5 retrys every 45 seconds
	for i in {1..5}
	do
		## Check network by pinging the gateway
		ping -q -c5 $Gateway_IP > /dev/null
		if [ $? -eq 0 ]
		then
			echo "Test $i: passed"
			echo "Test $i: passed" >> "$logfile"
		else
			echo "Test $i: failed"
			echo "Test $i: failed" >> "$logfile"
			fails=$((fails + 1))
		fi
		
		if [ $i -eq 5 ]
		then
			echo "Total fails: $fails"
			echo "Total fails: $fails" >> "$logfile"
			if [ $fails -eq 5 ]
			then
				echo "Restarting networking services"
				echo "Restarting networking services" >> "$logfile"

				## Restart network service when it fails 5 times consecutively 
				service networking stop
				sleep 5
				service networking start
			fi
		else
			sleep 90
		fi
	done
fi

