#!/bin/sh
SERVER=
PORT=1883
USERNAME=
PASSWORD=

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"sec_between_reading\":30,
\"fan_activate_temp\":99.0,
\"fan_activate_humid\":99.0
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant1.enabled\":true,
\"plant1.dry_value\":42.0,
\"plant1.wet_value\":40.0,
\"plant1.watering_duration_ms\":20000,
\"plant1.watering_grace_period_sec\":240
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant2.enabled\":true,
\"plant2.dry_value\":42.0,
\"plant2.wet_value\":40.0,
\"plant2.watering_duration_ms\":20000,
\"plant2.watering_grace_period_sec\":240
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant3.enabled\":true,
\"plant3.dry_value\":42.0,
\"plant3.wet_value\":40.0,
\"plant3.watering_duration_ms\":20000,
\"plant3.watering_grace_period_sec\":240
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant4.enabled\":true,
\"plant4.dry_value\":42.0,
\"plant4.wet_value\":40.0,
\"plant4.watering_duration_ms\":20000,
\"plant4.watering_grace_period_sec\":240
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant5.enabled\":true,
\"plant5.dry_value\":42.0,
\"plant5.wet_value\":40.0,
\"plant5.watering_duration_ms\":20000,
\"plant5.watering_grace_period_sec\":240
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant6.enabled\":true,
\"plant6.dry_value\":42.0,
\"plant6.wet_value\":40.0,
\"plant6.watering_duration_ms\":20000,
\"plant6.watering_grace_period_sec\":240
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant7.enabled\":true,
\"plant7.dry_value\":42.0,
\"plant7.wet_value\":40.0,
\"plant7.watering_duration_ms\":20000,
\"plant7.watering_grace_period_sec\":240
}" &&

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant8.enabled\":true,
\"plant8.dry_value\":0.0,
\"plant8.wet_value\":0.0,
\"plant8.watering_duration_ms\":20000,
\"plant8.watering_grace_period_sec\":86400
}"

sleep 1s &&

mosquitto_pub -h $SERVER -p $PORT -u $USERNAME -P $PASSWORD -t "/Drivhus/1/config" -m \
"{
\"plant9.enabled\":false,
\"plant10.enabled\":false,
\"plant11.enabled\":false,
\"plant12.enabled\":false,
\"plant13.enabled\":false,
\"plant14.enabled\":false,
\"plant15.enabled\":false
}"
