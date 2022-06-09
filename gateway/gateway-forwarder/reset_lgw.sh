#!/bin/sh

# This script is intended to be used on IoT Starter Kit platform, it performs
# the following actions:
#       - export/unpexort GPIO23 used to reset the SX1301 chip
#
# Usage examples:
#       ./reset_lgw.sh stop
#       ./reset_lgw.sh start

# The reset pin of SX1301 is wired with RPi GPIO7
# If used on another platform, the GPIO number can be given as parameter.
if [ -z "$2" ]; then 
    IOT_SK_SX1301_RESET_PIN=23
else
    IOT_SK_SX1301_RESET_PIN=$2
fi


WAIT_GPIO() {
    sleep 1
}

iot_sk_init() {
    # setup GPIO 23
    echo "$IOT_SK_SX1301_RESET_PIN" > /sys/class/gpio/export; WAIT_GPIO

    # set GPIO 23 as output
    echo "out" > /sys/class/gpio/gpio$IOT_SK_SX1301_RESET_PIN/direction; WAIT_GPIO

    # write output for SX1301 reset
    echo "1" > /sys/class/gpio/gpio$IOT_SK_SX1301_RESET_PIN/value; WAIT_GPIO
    echo "0" > /sys/class/gpio/gpio$IOT_SK_SX1301_RESET_PIN/value; WAIT_GPIO

    # set GPIO 23 as input
    echo "in" > /sys/class/gpio/gpio$IOT_SK_SX1301_RESET_PIN/direction; WAIT_GPIO
}

iot_sk_term() {
    # cleanup GPIO 23
    if [ -d /sys/class/gpio/gpio$IOT_SK_SX1301_RESET_PIN ]
    then
        echo "$IOT_SK_SX1301_RESET_PIN" > /sys/class/gpio/unexport; WAIT_GPIO
    fi
}

case "$1" in
    start)
    iot_sk_term
    iot_sk_init
    ;;
    stop)
    iot_sk_term
    ;;
    restart)
    iot_sk_init
    iot_sk_term
    ;;
    *)
    echo "Usage: $0 {start|stop} [<gpio number>]"
    exit 1
    ;;
esac

exit 0
