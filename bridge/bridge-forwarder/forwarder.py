#!/usr/bin/env python3
# -*- coding=utf-8 -*-

import paho.mqtt.client as mqtt
import redis
import json
# import pyzstd
import brotli
import yaml
import os
import sys
import re
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
    if path := os.environ.get(var + '_FILE'):
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


MQTT_HOST = os.environ.get('FOR_MQTT_HOST', '127.0.0.1')
MQTT_PORT = int(os.environ.get('FOR_MQTT_PORT', 1883))
# MQTT_USERNAME = get_fileenv('FOR_MQTT_USERNAME') or 'lorabridge'
# MQTT_PASSWORD = get_fileenv('FOR_MQTT_PASSWORD') or 'lorabridge'
MQTT_BASE_TOPIC = os.environ.get('FOR_MQTT_BASE_TOPIC', 'zigbee2mqtt')
# MQTT_SUB_TOPIC = os.environ.get('FOR_MQTT_SUB_TOPIC', 'lorabridge')
REDIS_HOST = os.environ.get('FOR_REDIS_HOST', "localhost")
REDIS_PORT = int(os.environ.get('FOR_REDIS_PORT', 6379))
REDIS_DB = int(os.environ.get('FOR_REDIS_DB', 0))
REDIS_LIST = os.environ.get('FOR_REDIS_LIST', "mylist")

DEVICE_REGEX = re.compile(f"^{re.escape(MQTT_BASE_TOPIC)}/0x[0-9a-fA-F]{{16}}")
# SUB_TOPIC_REGEX = re.compile(f"^{re.escape(MQTT_BASE_TOPIC)}/{MQTT_SUB_TOPIC}")

# DEVICE_CLASSES = ("None", "apparent_power", "aqi", "battery", "carbon_dioxide", "carbon_monoxide", "current", "date",
#                   "duration", "energy", "frequency", "gas", "humidity", "illuminance", "monetary", "nitrogen_dioxide",
#                   "nitrogen_monoxide", "nitrous_oxide", "ozone", "pm1", "pm10", "pm25", "power_factor", "power",
#                   "pressure", "reactive_power", "signal_strength", "sulphur_dioxide", "temperature", "timestamp",
#                   "volatile_organic_compounds", "voltage")

DEVICE_CLASSES = (
    'ac_frequency', 'action', 'action_group', 'angle', 'angle_axis', 'aqi', 'auto_lock', 'auto_relock_time',
    'away_mode', 'away_preset_days', 'away_preset_temperature', 'battery', 'battery_low', 'battery_voltage',
    'boost_time', 'button_lock', 'carbon_monoxide', 'child_lock', 'co2', 'co', 'comfort_temperature',
    'consumer_connected', 'contact', 'cover_position', 'cover_position_tilt', 'cover_tilt', 'cpu_temperature',
    'cube_side', 'current', 'current_phase_b', 'current_phase_c', 'deadzone_temperature', 'device_temperature', 'eco2',
    'eco_mode', 'eco_temperature', 'effect', 'energy', 'fan', 'flip_indicator_light', 'force', 'formaldehyd', 'gas',
    'hcho', 'holiday_temperature', 'humidity', 'illuminance', 'illuminance_lux', 'brightness_state', 'keypad_lockout',
    'led_disabled_night', 'light_brightness', 'light_brightness_color', 'light_brightness_colorhs',
    'light_brightness_colortemp', 'light_brightness_colortemp_color', 'light_brightness_colortemp_colorhs',
    'light_brightness_colortemp_colorxy', 'light_brightness_colorxy', 'light_colorhs', 'linkquality',
    'local_temperature', 'lock', 'lock_action', 'lock_action_source_name', 'lock_action_source_user', 'max_temperature',
    'max_temperature_limit', 'min_temperature', 'noise', 'noise_detected', 'occupancy', 'occupancy_level',
    'open_window', 'open_window_temperature', 'pm10', 'pm25', 'position', 'power', 'power_factor', 'power_apparent',
    'power_on_behavior', 'power_outage_count', 'power_outage_memory', 'presence', 'pressure',
    'programming_operation_mode', 'smoke', 'soil_moisture', 'sos', 'sound_volume', 'switch', 'switch_type',
    'switch_type_2', 'tamper', 'temperature', 'test', 'valve_position', 'valve_switch', 'valve_state',
    'valve_detection', 'vibration', 'voc', 'voltage', 'voltage_phase_b', 'voltage_phase_c', 'water_leak', 'warning',
    'week', 'window_detection', 'moving', 'x_axis', 'y_axis', 'z_axis', 'pincode', 'squawk')


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    logging.info("Connected with result code " + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(userdata['topic'] + '/#')


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    if re.match(DEVICE_REGEX, msg.topic):  # or (sub := re.match(SUB_TOPIC_REGEX, msg.topic)):
        topic = msg.topic.removeprefix(MQTT_BASE_TOPIC + "/")
        # if dev else MQTT_BASE_TOPIC + "/" + MQTT_SUB_TOPIC + "/")
        try:
            payload = json.loads(msg.payload)
            data = {}
            for key, value in payload.items():
                if key in DEVICE_CLASSES:
                    data[DEVICE_CLASSES.index(key)] = value
                else:
                    data[key] = value

            # message = topic + " " + yaml.dump({'temperature': json.loads(msg.payload)['temperature']})
            # import base64
            # tlist = topic.split("/", 1)
            # topic = tlist[0]
            # topic = int(topic, 16)
            # topic = (topic).to_bytes((topic).bit_length() // 8 + (0 if not (topic).bit_length() % 8 else 1),
            #                          byteorder='big')
            # topic = base64.base85encode(topic)
            # if tlist > 1:
            #     topic = topic + "/" + tlist[1]

            message = topic + " " + yaml.dump(data)
        except json.decoder.JSONDecodeError:
            # do nothing if zigbee2mqtt publishes garbage message
            return
        logging.info(message)
        # compressed = pyzstd.compress(bytes(message, "ascii"), level_or_option=pyzstd.compressionLevel_values.max)
        compressed = brotli.compress(bytes(message, encoding="ascii"), quality=11)
        userdata['r_client'].lpush(userdata['r_list'], compressed)


def main():
    logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s', level=logging.INFO)
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.connect(MQTT_HOST, MQTT_PORT, 60)
    r_client = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, db=REDIS_DB)
    client.user_data_set({"r_client": r_client, "topic": MQTT_BASE_TOPIC, "r_list": REDIS_LIST})

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_forever()


if __name__ == '__main__':
    main()
