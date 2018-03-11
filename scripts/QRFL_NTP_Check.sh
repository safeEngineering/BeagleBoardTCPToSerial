#!/bin/bash

# Release: V1R4 - 08/02/2018 - Joshua Crawford

logfile=/logs/web-app/public/log/QRFLlog_NTP.txt
settingsfile=/home/debian/QRFL_Network_Settings.json

## Check settings file exists
if [ -e "$settingsfile" ]
then
	echo "....................................................." > "$logfile"
	echo ".................. System NTP Check ................." >> "$logfile"
	echo "....................................................." >> "$logfile"
	
	## Time stamp
	Test_Time="$(date -R)"
	echo -e "NTP Test Start Time: $Test_Time"
	echo -e "NTP Test Start Time: $Test_Time" >> "$logfile"

	## Check NTP status
	NTP_status2="$(timedatectl status | grep '^NTP synchronized')"
	NTP_setting="$(cat $settingsfile | jq -r '.NTP_IP')"
	echo -e "NTP IP:$NTP_setting \n$NTP_status2"
	#echo -e "NTP IP:$NTP_setting \n$NTP_status2" >> "$logfile"
	NTP_Year="$(date +'%Y')"	

	## Restart NTP service if requred
	if [ "$NTP_status2" = "NTP synchronized: no" ] || [ $NTP_Year -le 2017 ]
	then
		echo -e "\nRestarting NTP..."
		echo -e "\nRestarting NTP..." >> "$logfile"
		systemctl restart systemd-timesyncd
	else
		echo "NTP is Synchronized"
		echo "NTP is Synchronized" >> "$logfile"
	fi
fi
