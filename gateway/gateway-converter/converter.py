#!/usr/bin/env python3
# -*- coding=utf-8 -*-

import paho.mqtt.client as mqtt
import json
import brotli
import yaml
import os
import sys
import base64
import logging
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader


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


MQTT_HOST = os.environ.get('CON_MQTT_HOST', '127.0.0.1')
MQTT_PORT = int(os.environ.get('CON_MQTT_PORT', 1883))
MQTT_USERNAME = get_fileenv('CON_MQTT_USERNAME') or 'lorabridge'
MQTT_PASSWORD = get_fileenv('CON_MQTT_PASSWORD') or 'lorabridge'
CHIRP_TOPIC = os.environ.get('CON_CHIRP_TOPIC', "chirp/stack")
DEV_MAN_TOPIC = os.environ.get('CON_DEV_MAN_TOPIC', "devicemanager")

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
    client.subscribe(userdata['chirp'] + '/#')


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    try:
        topic, data = brotli.decompress(base64.b64decode(json.loads(msg.payload)['data'])).split(b" ", maxsplit=1)
    # KeyError = filter chirpstack garbage
    except (json.decoder.JSONDecodeError, KeyError):
        # do nothing if zigbee2mqtt publishes garbage message
        return
    data = yaml.load(data, Loader=Loader)

    res = {}
    for key, value in data.items():
        try:
            res[DEVICE_CLASSES[int(key)]] = value
        except ValueError:
            res[key] = value

    # import base64
    # topic = topic.decode('utf-8')
    # tlist = topic.split("/", 1)
    # topic = tlist[0]
    # topic = base64.b85decode(topic)
    # topic = "0x{:016x}".format(int.from_bytes(value, byteorder="big"))
    # if tlist > 1:
    #     topic = topic + "/" + tlist[1]
    # topic = bytes(topic, "ascii")

    logging.info(DEV_MAN_TOPIC + "/" + topic.decode('utf-8') + " " + str(json.dumps(res)))
    client.publish(DEV_MAN_TOPIC + "/" + topic.decode('utf-8'), json.dumps(res))


def main():
    logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s', level=logging.INFO)
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.connect(MQTT_HOST, MQTT_PORT, 60)
    client.user_data_set({"chirp": CHIRP_TOPIC})

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_forever()


if __name__ == '__main__':
    main()