FROM ghcr.io/home-assistant/home-assistant:stable

VOLUME /config

RUN mkdir /config/.storage

COPY ./config/.storage/core.config_entries /config/.storage/core.config_entries
