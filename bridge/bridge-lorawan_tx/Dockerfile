FROM alpine:3.15 as build

WORKDIR /home/lora
# RUN echo "@community http://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories
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
RUN apk add --no-cache build-base hiredis-dev 
RUN apk add --no-cache wiringpi-dev --repository=http://dl-cdn.alpinelinux.org/alpine/edge/community

COPY . .

# WORKDIR /home/lora/lmic-rpi-lora-gps-hat/examples/transmit
WORKDIR /home/lora/rpi_loratx

RUN make

FROM alpine:3.15

# RUN echo "@community http://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories
RUN apk update
RUN apk add --no-cache hiredis
RUN apk add --no-cache wiringpi --repository=http://dl-cdn.alpinelinux.org/alpine/edge/community
#--repository=http://dl-cdn.alpinelinux.org/alpine/edge/community
WORKDIR /home/lora
COPY --from=build /home/lora/rpi_loratx/build/rpi_loratx.out /home/lora/rpi_loratx.out
# COPY --from=build /usr/lib/libwiringPi.so.* /usr/lib/
# COPY --from=build /usr/lib/libhiredis.so.* /usr/lib/

# ENTRYPOINT [ "tail", "-f", "/dev/null" ]
ENTRYPOINT [ "/home/lora/rpi_loratx.out" ]