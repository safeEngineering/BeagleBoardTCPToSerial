#!/bin/bash

## Stops logging services, clears logs, then restarts logging services

systemctl stop QRFL_E23_Comms.service
systemctl stop QRFL_Data.service
rm /logs/web-app/public/log/*-Default*
rm /logs/web-app/public/eventlogs/*-Default*
rm /logs/web-app/public/faultlogs/*-Default*
sleep 2
systemctl start QRFL_E23_Comms.service
systemctl start QRFL_Data.service
