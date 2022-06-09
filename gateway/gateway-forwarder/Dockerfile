FROM alpine:3.15 as build

WORKDIR /home/lora
RUN apk update
# alpine 3.15 install wiringpi 2.50-r0 which does not work:
# 000000000 HAL: Initializing ...
# HAL: There is an issue with the SPI communication to the radio module.
# HAL: Make sure that 
# HAL: * The radio module is attached to your Raspberry Pi
# HAL: * The power supply provides enough power
# HAL: * SPI is enabled on your Raspberry Pi. Use the tool "raspi-config" to enable it.
# 000000001 HAL: Failed. Aborting.

# alpine edge install wiringpi 2.61-r0 which works with --privileged
RUN apk add --no-cache build-base linux-headers
# RUN apk add --no-cache wiringpi-dev --repository=http://dl-cdn.alpinelinux.org/alpine/edge/community

# COPY . .
COPY lora_pkt_fwd/ lora_pkt_fwd/ 
COPY libloragw/ libloragw/
COPY VERSION VERSION

WORKDIR /home/lora/libloragw
RUN make
WORKDIR /home/lora/lora_pkt_fwd
RUN rm global_conf.json
RUN make

FROM alpine:3.15
WORKDIR /home/lora

# RUN apk update
# RUN apk add --no-cache hiredis
# RUN apk add --no-cache wiringpi --repository=http://dl-cdn.alpinelinux.org/alpine/edge/community
# #--repository=http://dl-cdn.alpinelinux.org/alpine/edge/community
# WORKDIR /home/lora
# COPY --from=build /home/lora/lmic-rpi-lora-gps-hat/examples/transmit/build/transmit.out /home/lora/transmit.out

COPY --from=build /home/lora/lora_pkt_fwd/lora_pkt_fwd /home/lora/lora_pkt_fwd/lora_pkt_fwd

# ENTRYPOINT [ "tail", "-f", "/dev/null" ]
COPY reset_lgw.sh /home/lora/
COPY lora_pkt_fwd/global_conf.json lora_pkt_fwd/global_conf.json
WORKDIR /home/lora/lora_pkt_fwd

ENTRYPOINT ["/bin/sh", "-c"]
# ENTRYPOINT ["/bin/sh", "-c", "/home/lora/lora_pkt_fwd/lora_pkt_fwd"]
# "/home/lora/lora_pkt_fwd/lora_pkt_fwd" 
CMD [ "/home/lora/reset_lgw.sh start && /home/lora/lora_pkt_fwd/lora_pkt_fwd" ]