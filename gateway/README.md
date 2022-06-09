# PyGateway

## Purpose
This docker stack is to be used on the LoRaWAN Gateway.
It consists of following containers:
- Redis2MQTT Helper (only for development in `docker-compose.dev.yml`, takes redis list entries and publishes them as MQTT topic)
- Python Converter
  - subscribes to MQTT Topic (provided by ChirpStack), decompresses the message with brotli, converts yaml payload to json and publishes the entry with the original MQTT topic  

## Setup
- clone this repository
- copy `env.example` to `.env`
- modify variables in `.env` as necessary
- variables in `.env` are used in `docker-compose.yml` file
- __change the network it attaches to in `docker-compose.yml`__
  - use the ChirpStack docker network
- build containers with `docker-compose build`
- start containers with `docker-compose up -d`

### Dev Setup
Use `docker-compose.dev.yml` file:
```bash
docker-compose -f docker-compose.dev.yml build
docker-compose -f docker-compose.dev.yml up -d
```
