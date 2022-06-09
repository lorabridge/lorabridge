#!/usr/bin/env python3
# -*- coding=utf-8 -*-

import paho.mqtt.client as mqtt
import json
import os
import sys
import redis
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


MQTT_HOST = os.environ.get('DEV_MQTT_HOST', '127.0.0.1')
MQTT_PORT = int(os.environ.get('DEV_MQTT_PORT', 1883))
MQTT_USERNAME = get_fileenv('DEV_MQTT_USERNAME') or 'lorabridge'
MQTT_PASSWORD = get_fileenv('DEV_MQTT_PASSWORD') or 'lorabridge'
DEV_MAN_TOPIC = os.environ.get('DEV_DEV_MAN_TOPIC', "devicemanager")
REDIS_HOST = os.environ.get('DEV_REDIS_HOST', "localhost")
REDIS_PORT = int(os.environ.get('DEV_REDIS_PORT', 6379))
REDIS_DB = int(os.environ.get('DEV_REDIS_DB', 0))
DISCOVERY_TOPIC = os.environ.get('DEV_DISCOVERY_TOPIC', 'lorabridge/discovery')
STATE_TOPIC = os.environ.get('DEV_STATE_TOPIC', 'lorabridge/state')

REDIS_SEPARATOR = ":"
REDIS_PREFIX = "lorabridge:devman"


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    logging.info("Connected with result code " + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(userdata['topic'] + '/#')


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    rclient = userdata['r_client']
    data = json.loads(msg.payload)

    if not rclient.exists(
        (rkey := REDIS_SEPARATOR.join([REDIS_PREFIX, (ieee := msg.topic.removeprefix(DEV_MAN_TOPIC + "/"))]))):
        index = rclient.incr(REDIS_SEPARATOR.join([REDIS_PREFIX, "dev_index"]))
        dev_data = {"ieee": ieee, "id": index, "measurement": json.dumps(list(data.keys()))}
        rclient.hset(rkey, mapping=dev_data)
        rclient.sadd(REDIS_SEPARATOR.join([REDIS_PREFIX, "devices"]), rkey)
        dev_data['measurement'] = json.loads(dev_data['measurement'])
        dev_data.update({"value": data})
        client.publish(DISCOVERY_TOPIC, json.dumps(dev_data))
        client.publish(STATE_TOPIC, json.dumps(dev_data))
        # discovery
    else:
        # state update
        dev_data = rclient.hgetall(rkey)
        dev_data['measurement'] = json.loads(dev_data['measurement'])
        dev_data.update({"value": data})
        client.publish(STATE_TOPIC, json.dumps(dev_data))


def main():
    logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s', level=logging.INFO)
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.connect(MQTT_HOST, MQTT_PORT, 60)
    client.user_data_set({"topic": DEV_MAN_TOPIC})

    r_client = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, db=REDIS_DB, decode_responses=True)
    client.user_data_set({"r_client": r_client, "topic": DEV_MAN_TOPIC})

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_forever()


if __name__ == '__main__':
    main()