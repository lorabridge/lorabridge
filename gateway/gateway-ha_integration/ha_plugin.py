#!/usr/bin/env python3
# -*- coding=utf-8 -*-

import paho.mqtt.client as mqtt
import json
import os
import sys
import yaml
import logging


def get_fileenv(var: str):
    """Tries to read the provided env var name + _FILE first and read the file at the path of env var value.
    If that fails, it looks at /run/secrets/<env var>, otherwise uses the env var itself.
    Args:
        var (str): Name of the provided environment variable.

    Returns:
        Content of the environment variable file if exists, or the value of the environment variable.
        None if the environment variable does not exist.
    """
    path = os.environ.get(var + '_FILE')

    if path:
        with open(path) as file:
            return file.read().strip()
    else:
        try:
            with open(os.path.join('run', 'secrets', var.lower())) as file:
                return file.read().strip()
        except IOError:
            # mongo username needs to be string and not empty (fix for sphinx)
            if 'sphinx' in sys.modules:
                return os.environ.get(var, 'fail')
            else:
                return os.environ.get(var)


MQTT_HOST = os.environ.get('HA_MQTT_HOST', '127.0.0.1')
MQTT_PORT = int(os.environ.get('HA_MQTT_PORT', 1883))
MQTT_USERNAME = get_fileenv('HA_MQTT_USERNAME') or 'lorabridge'
MQTT_PASSWORD = get_fileenv('HA_MQTT_PASSWORD') or 'lorabridge'
LORABRIDGE_DISCOVERY_TOPIC = os.environ.get('HA_DISCOVERY_TOPIC', 'lorabridge/discovery')
LORABRIDGE_STATE_TOPIC = os.environ.get('HA_STATE_TOPIC', 'lorabridge/state')

# DEVICE_CLASSES = ("None", "apparent_power", "aqi", "battery", "carbon_dioxide", "carbon_monoxide", "current", "date",
#                   "duration", "energy", "frequency", "gas", "humidity", "illuminance", "monetary", "nitrogen_dioxide",
#                   "nitrogen_monoxide", "nitrous_oxide", "ozone", "pm1", "pm10", "pm25", "power_factor", "power",
#                   "pressure", "reactive_power", "signal_strength", "sulphur_dioxide", "temperature", "timestamp",
#                   "volatile_organic_compounds", "voltage")
# see https://www.home-assistant.io/integrations/sensor/#device-class

# extracted from https://github.com/Koenkk/zigbee2mqtt/blob/master/lib/extension/homeassistant.ts#L535
DEVICE_MAPPINGS = yaml.load(open("device_mappings.yaml"), Loader=yaml.Loader)

UNITS = yaml.load(open("units.yaml"), Loader=yaml.Loader)


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    logging.info("Connected with result code " + str(rc))
    logging.info("MQTT_PORT: " + str(MQTT_PORT))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(userdata['lorabridge_state'] + '/#')
    client.subscribe(userdata['lorabridge_discovery'] + '/#')


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload)
    except json.decoder.JSONDecodeError:
        # do nothing if zigbee2mqtt publishes garbage message
        return

    if msg.topic == LORABRIDGE_DISCOVERY_TOPIC:
        convert_to_ha_discovery(data, client)

    if msg.topic == LORABRIDGE_STATE_TOPIC:
        convert_to_ha_state(data, client)


def convert_to_ha_discovery(data, client):

    # measurement_type = data["measurement"]
    device_id = data["id"]

    # if measurement_type != "temperature":
    #     print("Unsupported measurement type!")
    #     return

    for measurement_type in data["measurement"]:

        # default sensor type
        sensor_type = "sensor"

        if measurement_type in DEVICE_MAPPINGS:
            sensor_type = DEVICE_MAPPINGS[measurement_type]["sensor_type"]
        elif type(data["value"][measurement_type]) is bool:
            sensor_type = "binary_sensor"

        ha_discovery_topic = "homeassistant/" + sensor_type + "/lorabridge_sensor" + str(
            device_id) + "_" + measurement_type + "/config"

        ha_discovery_msg = {}
        ha_discovery_msg["name"] = "lorabridge_sensor" + str(device_id) + "_" + measurement_type

        try:
            ha_discovery_msg["device_class"] = DEVICE_MAPPINGS[measurement_type]['device_class']
            ha_discovery_msg["unit_of_measurement"] = UNITS[DEVICE_MAPPINGS[measurement_type]['device_class']]
        except KeyError:
            pass

        # if measurement_type not in DEVICE_CLASSES:
        #     ha_discovery_msg["device_class"] = None
        # else:
        #     ha_discovery_msg["device_class"] = measurement_type
        #     ha_discovery_msg["unit_of_measurement"] = DEVICE_CLASSES[measurement_type]
        ha_discovery_msg["state_topic"] = "homeassistant/sensor/lorabridge_sensor" + str(device_id) + "/state"
        ha_discovery_msg["value_template"] = "{{ value_json." + measurement_type + "}}"

        # Publish with retain
        client.publish(ha_discovery_topic, json.dumps(ha_discovery_msg), 0, retain=True)


def convert_to_ha_state(data, client):
    ha_state_msg = {}

    measurement_type = data["measurement"]
    # measurement_value = data["value"]
    device_id = data["id"]

    # sensor_type = "sensor"

    #if type(data["value"]) is bool:
    #    sensor_type = "binary_sensor"

    # if measurement_type != "temperature":
    #     print("Unsupported measurement type!")
    #     return

    ha_state_topic = "homeassistant/sensor/lorabridge_sensor" + str(device_id) + "/state"

    # ha_state_msg[measurement_type] = measurement_value

    for val in data['value']:

        #if type(data['value'][val]) is bool:
        #    data['value'][val] = 'ON' if data['value'][val] else 'OFF'

        # Map Z2M measurements to HA MQTT measurements

        if val in DEVICE_MAPPINGS:
            if "value_mapping" in DEVICE_MAPPINGS[val] and data['value'][val] in DEVICE_MAPPINGS[val]["value_mapping"]:
                data['value'][val] = DEVICE_MAPPINGS[val]["value_mapping"][data['value'][val]]

    client.publish(ha_state_topic, json.dumps(data['value']), 0, retain=True)


def main():
    logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s', level=logging.INFO)
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.user_data_set({
        "lorabridge_discovery": LORABRIDGE_DISCOVERY_TOPIC,
        "lorabridge_state": LORABRIDGE_STATE_TOPIC
    })
    client.connect(MQTT_HOST, MQTT_PORT, 60)

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_forever()


if __name__ == '__main__':
    main()
