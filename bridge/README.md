# PyForwarder

## Purpose
This docker stack is to be used on the LoRaWAN Bridge.
It consists of following containers:
- Eclipse Mosqitto
- Python MQTT Generator (modified, original on [GitHub](https://gist.github.com/marianoguerra/be216a581ef7bc23673f501fdea0e15a))
- Python Forwarder
  - subscribes to MQTT Topic, converts json to yaml, compresses with brotli and pushes to Redis list

## Setup
- clone this repository
- copy `env.example` to `.env`
- modify variables in `.env` as necessary
- variables in `.env` are used in `docker-compose.yml` file
- build containers with `docker-compose build`
- start containers with `docker-compose up -d`

## Notes
This docker-compose.yml create an attachable network `lorabridgenet`. Other containers can be attached to this network, e.g. the `zigbee2mqtt` container.


