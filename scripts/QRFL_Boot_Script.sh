#!/bin/bash

# Release: V1R11 - 08/02/2018 - Joshua Crawford

## Define Used file locations
NetworkSettings=/home/debian/QRFL_Network_Settings.json
NetworkConfiguration=/home/debian/QRFL_Network_Configuration.json
NetworkLog=/logs/web-app/public/log/QRFLlog_Boot_1.txt

## Circulate old boot logs
if [ -e "/logs/web-app/public/log/QRFLlog_Boot_4.txt" ]
then
	rm /logs/web-app/public/log/QRFLlog_Boot_4.txt
fi

if [ -e "/logs/web-app/public/log/QRFLlog_Boot_3.txt" ]
then
	cp /logs/web-app/public/log/QRFLlog_Boot_3.txt /logs/web-app/public/log/QRFLlog_Boot_4.txt
	rm /logs/web-app/public/log/QRFLlog_Boot_3.txt
fi

if [ -e "/logs/web-app/public/log/QRFLlog_Boot_2.txt" ]
then
	cp /logs/web-app/public/log/QRFLlog_Boot_2.txt /logs/web-app/public/log/QRFLlog_Boot_3.txt
	rm /logs/web-app/public/log/QRFLlog_Boot_2.txt
fi

if [ -e "/logs/web-app/public/log/QRFLlog_Boot_1.txt" ]
then
	cp /logs/web-app/public/log/QRFLlog_Boot_1.txt /logs/web-app/public/log/QRFLlog_Boot_2.txt
	rm /logs/web-app/public/log/QRFLlog_Boot_1.txt
fi




echo "........................................................."
echo "........................ Settings ......................."
echo "........................................................."
echo "........................................................." > "$NetworkLog"
echo "........................ Settings ......................." >> "$NetworkLog"
echo "........................................................." >> "$NetworkLog"

echo -e "PID:  $$" 
echo -e "PID: $$" >> "$NetworkLog"

#### If the network settings json file exists, load settings.
if [ -e "$NetworkSettings" ]
then
	## extract current network configuration of the BeagleBone
	IP_setting="$(cat $NetworkSettings | jq -r '.IP')"
	mask_setting="$(cat $NetworkSettings | jq -r '.Mask')"
	gateway_setting="$(cat $NetworkSettings | jq -r '.Gateway')"
	DNS_setting="$(cat $NetworkSettings | jq -r '.DNS')"
	NTP_setting="$(cat $NetworkSettings | jq -r '.NTP_IP')"
	
	
	## Check settings are a valid format
	IP_valid=1
	mask_valid=1
	gateway_valid=1
	DNS_valid=1
	NTP_valid=1

	if expr "$IP_setting" : '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$' >/dev/null; then
  		for i in 1 2 3 4; do
    		if [ $(echo "$IP_setting" | cut -d. -f$i) -gt 255 ]; then
      			let "IP_valid = 0"
    		fi
  		done
	else
  		let "IP_valid = 0"
	fi

	if expr "$mask_setting" : '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$' >/dev/null; then
  		for i in 1 2 3 4; do
    		if [ $(echo "$mask_setting" | cut -d. -f$i) -gt 255 ]; then
      			let "mask_valid = 0"
    		fi
  		done
	else
  		let "mask_valid = 0"
	fi

	if expr "$gateway_setting" : '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$' >/dev/null; then
  		for i in 1 2 3 4; do
    		if [ $(echo "$gateway_setting" | cut -d. -f$i) -gt 255 ]; then
      			let "gateway_valid = 0"
    		fi
  		done
	else
  		let "gateway_valid = 0"
	fi

	if expr "$DNS_setting" : '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$' >/dev/null; then
  		for i in 1 2 3 4; do
    		if [ $(echo "$DNS_setting" | cut -d. -f$i) -gt 255 ]; then
      			let "DNS_valid = 0"
    		fi
  		done
	else
  		let "DNS_valid = 0"
	fi

	if expr "$NTP_setting" : '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$' >/dev/null; then
  		for i in 1 2 3 4; do
    		if [ $(echo "$NTP_setting" | cut -d. -f$i) -gt 255 ]; then
      			let "NTP_valid = 0"
    		fi
  		done
	else
  		let "NTP_valid = 0"
	fi

	settings_valid=$(($IP_valid+$mask_valid+$gateway_valid+$DNS_valid+$NTP_valid))

	## Print settings to log
	echo -e "IP Address Setting:        $IP_setting"
	echo "Netmask Setting:           $mask_setting"
	echo "Default Gateway Setting:   $gateway_setting"
	echo "Name Server Setting:       $DNS_setting"
	echo "NTP Server Setting:        $NTP_setting"
	echo -e "IP Address Setting:        $IP_setting" >> "$NetworkLog"
	echo "Netmask Setting:           $mask_setting" >> "$NetworkLog"
	echo "Default Gateway Setting:   $gateway_setting" >> "$NetworkLog"
	echo "Name Server Setting:       $DNS_setting" >> "$NetworkLog"
	echo "NTP Server Setting:        $NTP_setting" >> "$NetworkLog"

else
	echo -e "\nNo QRFL_Ethernet_Settings.json File Found!"
	echo -e "\nNo QRFL_Ethernet_Settings.json File Found!" >> "$NetworkLog"
	settings_valid=0
fi




echo -e "\n........................................................." 
echo ".................... Unit Configuration ................." 
echo "........................................................." 
echo -e "\n........................................................." >> "$NetworkLog"
echo ".................... Unit Configuration ................." >> "$NetworkLog"
echo "........................................................." >> "$NetworkLog"

## Extract current network configurations from Beaglebone.
IP_addr="$(/sbin/ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}')"
Mask_addr="$(ifconfig eth0 | awk -F: '/Mask:/{print $4}')"
Gateway_addr="$(route -n | grep 'UG[ \t]' | awk '{print $2}')"
DNS_addr="$(cat /etc/resolv.conf | grep -i '^nameserver' | head -n1 | cut -d ' ' -f2)"
NTP_addr="$(cat /var/local/timesyncd.conf | grep -i '^NTP=' | head -n1 | cut -f2- -d=)"

## Print current configration to Log
echo "Current IP Address:        $IP_addr"
echo "Current Netmask:           $Mask_addr"
echo "Current Default Gateway:   $Gateway_addr"
echo "Current Name Server:       $DNS_addr"
echo "Current NTP IP Address:	 $NTP_addr"
echo "Current IP Address:        $IP_addr" >> "$NetworkLog"
echo "Current Netmask:           $Mask_addr" >> "$NetworkLog"
echo "Current Default Gateway:   $Gateway_addr" >> "$NetworkLog"
echo "Current Name Server:       $DNS_addr" >> "$NetworkLog"
echo "Current NTP IP Address:	    $NTP_addr" >> "$NetworkLog"

## timestamp the boot time
initial_time="$(date -R)"

#### Change unit configuration, if the settings file does not match the current setup
if [ $settings_valid == 5 ] 
then
	if [ $IP_addr != $IP_setting ] || [ -z "$IP_addr" ] || [ $Mask_addr != $mask_setting ] || [ $Gateway_addr != $gateway_setting ] || [ $DNS_addr != $DNS_setting ] || [ $NTP_addr != $NTP_setting ]
	then
		echo -e "\nSettings are valid and do not match system configuration!\nReconfiguring..."
		echo -e "\nSettings are valid and do not match system configuration!\nReconfiguring..." >> "$NetworkLog"

		## Stop services that settings are being changed on
		systemctl stop systemd-timesyncd
		service networking stop
		sleep 2

		## Change settings
		sed -i "s/^    address.*/    address $IP_setting/" /var/local/aurizon_network
		sed -i "s/^    netmask.*/    netmask $mask_setting/" /var/local/aurizon_network
		sed -i "s/^    gateway.*/    gateway $gateway_setting/" /var/local/aurizon_network
		sed -i "s/^    dns-nameservers.*/    dns-nameservers $DNS_setting/" /var/local/aurizon_network
		sed -i "s/^NTP=.*/NTP=$NTP_setting/" /var/local/timesyncd.conf

		## Start network and time server services
		service networking start
		systemctl start systemd-timesyncd
		timedatectl set-ntp true
		sleep 5

		## Get new configured Settings
		IP_new="$(/sbin/ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}')"
		Mask_new="$(ifconfig eth0 | awk -F: '/Mask:/{print $4}')"
		Gateway_new="$(route -n | grep 'UG[ \t]' | awk '{print $2}')"
		DNS_new="$(cat /etc/resolv.conf | grep -i '^nameserver' | head -n1 | cut -d ' ' -f2)"
		NTP_new="$(cat /var/local/timesyncd.conf | grep -i '^NTP=' | head -n1 | cut -f2- -d=)"

		echo -e "\nNew IP Address:        $IP_new"
		echo "New Netmask:           $Mask_new"
		echo "New Default Gateway:   $Gateway_new"
		echo "New Name Server:       $DNS_new"
		echo "New NTP IP Address:	 $NTP_new"
		echo -e "\nNew IP Address:        $IP_new" >> "$NetworkLog"
		echo "New Netmask:           $Mask_new" >> "$NetworkLog"
		echo "New Default Gateway:   $Gateway_new" >> "$NetworkLog"
		echo "New Name Server:       $DNS_new" >> "$NetworkLog"
		echo "New NTP IP Address:	    $NTP_new" >> "$NetworkLog"

		## Write JSON File with current system configuration
		echo "{\"DNS_Configuration\":\"$DNS_new\",\"Gateway_Configuration\":\"$Gateway_new\",\"IP_Configuration\":\"$IP_new\",\"Mask_Configuration\":\"$Mask_new\",\"NTP_Configuration\":\"$NTP_new\"}" > "$NetworkConfiguration"

	else
		echo -e  "\nSettings match system configuration, no changes required."
		echo -e  "\nSettings match system configuration, no changes required." >> "$NetworkLog"
	fi
else
	echo -e "\nSettings are not all valid ($settings_valid / 5), \nPlease reconfigure network settings on Control Board." 
	echo -e "\nSettings are not all valid ($settings_valid / 5), \nPlease reconfigure network settings on Control Board." >> "$NetworkLog" 
fi




echo -e "\n........................................................." 
echo ".....................QRFL NTP ..........................."
echo "........................................................."
echo -e "\n........................................................." >> "$NetworkLog"
echo ".....................QRFL NTP ..........................." >> "$NetworkLog"
echo "........................................................." >> "$NetworkLog"

## Check the NTP is running

echo "Boot Time: $initial_time"
echo "Boot Time: $initial_time" >> "$NetworkLog"
corrected_boot_time="$(date -R)"
echo "Corrected Boot Time: $corrected_boot_time"
echo "Corrected Boot Time: $corrected_boot_time" >> "$NetworkLog"
NTP_status1="$(timedatectl status | grep '^ Network')"
NTP_status2="$(timedatectl status | grep '^NTP synchronized')"
echo "$NTP_status1"
echo "$NTP_status2" 
echo "$NTP_status1" >> "$NetworkLog"
echo "$NTP_status2" >> "$NetworkLog"





