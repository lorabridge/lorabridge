/**
 * Author: Dragino 
 * Date: 16/01/2018
 * 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.dragino.com
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include <linux/spi/spidev.h>

#include <pthread.h>

#include "radio.h"

static const uint8_t rxlorairqmask[] = {
    [RXMODE_SINGLE] = IRQ_LORA_RXDONE_MASK|IRQ_LORA_RXTOUT_MASK|IRQ_LORA_CRCERR_MASK,
    [RXMODE_SCAN]   = IRQ_LORA_RXDONE_MASK|IRQ_LORA_CRCERR_MASK,
    [RXMODE_RSSI]   = 0x00,
};


/*
 * Reserve a GPIO for this program's use.
 * @gpio the GPIO pin to reserve.
 * @return true if the reservation was successful.
 */
static bool gpio_reserve(int gpio) {
    int fd; /* File descriptor for GPIO controller class */
    char buf[3]; /* Write buffer */

    /* Try to open GPIO controller class */
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
        fprintf(stderr, "ERROR~ gpio_reserve: could not open /sys/class/gpio/export\r\n");
        return false;
    }

    /* Prepare buffer */
    snprintf(buf, 3, "%d", gpio);

    /* Try to reserve GPIO */
    if (write(fd, buf, strlen(buf)) < 0) {
        close(fd);
        fprintf(stderr, "ERROR~ gpio_reserve: could not write '%s' to /sys/class/gpio/export\r\n", buf);
        return false;
    }

    /* Close the GPIO controller class */
    if (close(fd) < 0) {
        fprintf(stderr, "ERROR~ gpio_reserve: could not close /sys/class/gpio/export\r\n");
        return false;
    }

    /* Success */
    return true;
}

/*
 * Release a GPIO after use.
 * @gpio the GPIO pin to release.
 * @return true if the release was successful.
 */
static bool gpio_release(int gpio) {
    int fd; /* File descriptor for GPIO controller class */
    char buf[3]; /* Write buffer */

    /* Try to open GPIO controller class */
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
       fprintf(stderr, "ERROR~ gpio_release: could not open /sys/class/gpio/unexport\r\n");
       return false;
    }

    /* Prepare buffer */
    snprintf(buf, 3, "%d", gpio);

    /* Try to release GPIO */
    if (write(fd, buf, strlen(buf)) < 0) {
        fprintf(stderr, "ERROR~ gpio_release: could not write /sys/class/gpio/unexport\r\n");
        return false;
    }

    /* Close the GPIO controller class */
    if (close(fd) < 0) {
        fprintf(stderr, "ERROR~ gpio_release: could not close /sys/class/gpio/unexport\r\n");
        return false;
    }

    /* Success */
    return true;
}

/*
 * Set the direction of the GPIO port.
 * @gpio the GPIO pin to release.
 * @direction the direction of the GPIO port.
 * @return true if the direction could be successfully set.
 */
static bool gpio_set_direction(int gpio, int direction) {
    int fd; /* File descriptor for GPIO port */
    char buf[33]; /* Write buffer */

    /* Make the GPIO port path */
    snprintf(buf, 33, "/sys/class/gpio/gpio%d/direction", gpio);

    /* Try to open GPIO port for writing only */
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
        return false;
    }

    /* Set the port direction */
    if (direction == GPIO_OUT) {
        if (write(fd, "out", 3) < 0) {
            return false;
        }
    } else {
        if (write(fd, "in", 2) < 0) {
            return false;
        }
    }

    /* Close the GPIO port */
    if (close(fd) < 0) {
        return false;
    }

    /* Success */
    return true;
}

/*
 * Set the state of the GPIO port.
 * @gpio the GPIO pin to set the state for.
 * @state 1 or 0
 * @return true if the state change was successful.
 */
static bool gpio_set_state(int gpio, int state) {
    int fd; /* File descriptor for GPIO port */
    char buf[29]; /* Write buffer */

    /* Make the GPIO port path */
    snprintf(buf, 29, "/sys/class/gpio/gpio%d/value", gpio);

    /* Try to open GPIO port */
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        /* The file could not be opened */
        return false;
    }

    /* Set the port state */
    if (write(fd, (state == 1 ? "1" : "0"), 1) < 0) {
        return false;
    }

    /* Close the GPIO port */
    if (close(fd) < 0) {
        return false;
    }

    /* Success */
    return true;
}

/*
 * Get the state of the GPIO port.
 * @gpio the gpio pin to get the state from.
 * @return GPIO_HIGH if the pin is HIGH, GPIO_LOW if the pin is low. GPIO_ERR
 * when an error occured. 
 */
static int gpio_get_state(int gpio) {
    int fd;             /* File descriptor for GPIO port */
    char buf[29];       /* Write buffer */
    char port_state; /* Character indicating the port state */
    int state; /* API integer indicating the port state */

    /* Make the GPIO port path */
    snprintf(buf, 29, "/sys/class/gpio/gpio%d/value", gpio);

    /* Try to open GPIO port */
    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        /* The file could not be opened */
        fprintf(stderr, "ERROR~ gpio_get_state: could not open /sys/class/gpio/gpio%d/value\r\n", gpio);
        return LOW;
    }

    /* Read the port state */
    if (read(fd, &port_state, 1) < 0) {
        close(fd);
        fprintf(stderr, "ERROR~ gpio_get_state: could not read /sys/class/gpio/gpio%d/value\r\n", gpio);
        return LOW;
    }

    /* Translate the port state into API state */
    state = port_state == '1' ? HIGH : LOW;

    /* Close the GPIO port */
    if (close(fd) < 0) {
        fprintf(stderr, "ERROR~ gpio_get_state: could not close /sys/class/gpio/gpio%d/value\r\n", gpio);
        return LOW;
    }

    /* Return the state */
    return state;
}

void wait_ms(unsigned long a) {
    struct timespec dly;
    struct timespec rem;

    dly.tv_sec = a / 1000;
    dly.tv_nsec = ((long)a % 1000) * 1000000;

    //MSG("NOTE dly: %ld sec %ld ns\n", dly.tv_sec, dly.tv_nsec);

    if((dly.tv_sec > 0) || ((dly.tv_sec == 0) && (dly.tv_nsec > 100000))) {
        clock_nanosleep(CLOCK_MONOTONIC, 0, &dly, &rem);
        //MSG("NOTE remain: %ld sec %ld ns\n", rem.tv_sec, rem.tv_nsec);
    }
    return;
}

void wait_us(unsigned long a) {
    struct timespec dly;
    struct timespec rem;

    dly.tv_sec = a / 1000000;
    dly.tv_nsec = ((long)a % 1000000) * 1000;

    //MSG("NOTE dly: %ld sec %ld ns\n", dly.tv_sec, dly.tv_nsec);

    if((dly.tv_sec > 0) || ((dly.tv_sec == 0) && (dly.tv_nsec > 100000))) {
        clock_nanosleep(CLOCK_MONOTONIC, 0, &dly, &rem);
        //MSG("NOTE remain: %ld sec %ld ns\n", rem.tv_sec, rem.tv_nsec);
    }
    return;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void digitalWrite(int gpio, int state)
{
    gpio_reserve(gpio);
    gpio_set_direction(gpio, GPIO_OUT);
    gpio_set_state(gpio, state);
    gpio_release(gpio);
} 

/*
 * Read the state of the port. The port can be input
 * or output.
 * @gpio the GPIO pin to read from.
 * @return GPIO_HIGH if the pin is HIGH, GPIO_LOW if the pin is low. Output
 * is -1 when an error occurred.
 */
int digitalRead(int gpio) {
    int state; /* The port state */

    /* Reserve the port */
    if (!gpio_reserve(gpio)) {
        return -1;
    }

    /* Read the port */
    state = gpio_get_state(gpio);

    if (!gpio_release(gpio)) {
        return -1;
    }

    /* Return the port state */
    return state;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void selectreceiver(int pin)
{
    digitalWrite(pin, LOW);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void unselectreceiver(int pin)
{
    digitalWrite(pin, HIGH);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple write */
static int lgw_spi_w(uint8_t spidev, uint8_t address, uint8_t data) {
    uint8_t out_buf[3];
    uint8_t command_size;
    struct spi_ioc_transfer k;
    int a;

    /* check input variables */
    if ((address & 0x80) != 0) {
        fprintf(stderr, "ERROR~ WARNING: SPI address > 127\n");
    }

    /* prepare frame to be sent */
    out_buf[0] = WRITE_ACCESS | (address & 0x7F);
    out_buf[1] = data;
    command_size = 2;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.len = command_size;
    k.speed_hz = SPI_SPEED;
    k.cs_change = 0;
    k.bits_per_word = 8;
    a = ioctl(spidev, SPI_IOC_MESSAGE(1), &k);

    /* determine return code */
    if (a != (int)k.len) {
        fprintf(stderr, "ERROR~ ERROR~ SPI WRITE FAILURE\n");
        return -1;
    } else {
        //printf("Note: SPI write success\n");
        return 0;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple read */
static int lgw_spi_r(uint8_t spidev, uint8_t address, uint8_t *data) {
    uint8_t out_buf[3];
    uint8_t command_size;
    uint8_t in_buf[10];
    struct spi_ioc_transfer k;
    int a;

    /* check input variables */
    if ((address & 0x80) != 0) {
        fprintf(stderr, "ERROR~ WARNING: SPI address > 127\n");
    }

    /* prepare frame to be sent */
    out_buf[0] = READ_ACCESS | (address & 0x7F);
    out_buf[1] = 0x00;
    command_size = 2;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.rx_buf = (unsigned long) in_buf;
    k.len = command_size;
    k.cs_change = 0;
    a = ioctl(spidev, SPI_IOC_MESSAGE(1), &k);

    /* determine return code */
    if (a != (int)k.len) {
        fprintf(stderr, "ERROR~ SPI READ FAILURE\n");
        return -1;
    } else {
        //printf("Note: SPI read success\n");
        *data = in_buf[command_size - 1];
        return 0;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static uint8_t readReg(uint8_t spidev, uint8_t addr)
{
    uint8_t data = 0x00;

    lgw_spi_r(spidev, addr, &data);

    return data;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void writeReg(uint8_t spidev, uint8_t addr, uint8_t value)
{
    lgw_spi_w(spidev, addr, value);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void opmode (uint8_t spidev, uint8_t mode) {
    writeReg(spidev, REG_OPMODE, (readReg(spidev, REG_OPMODE) & ~OPMODE_MASK) | mode);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void opmodeLora(uint8_t spidev) {
    uint8_t u = OPMODE_LORA | 0x8; // TBD: sx1276 high freq
    writeReg(spidev, REG_OPMODE, u);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void setpower(uint8_t spidev, uint8_t pw) {
    writeReg(spidev, REG_PADAC, 0x87);
    if (pw < 5) pw = 5;
    if (pw > 20) pw = 20;
    writeReg(spidev, REG_PACONFIG, (uint16_t)(0x80 | ((pw - 5) & 0x0f)));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setfreq(uint8_t spidev, long frequency)
{
    uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

    writeReg(spidev, REG_FRF_MSB, (uint8_t)(frf >> 16));
    writeReg(spidev, REG_FRF_MID, (uint8_t)(frf >> 8));
    writeReg(spidev, REG_FRF_LSB, (uint8_t)(frf >> 0));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setsf(uint8_t spidev, int sf)
{
    if (sf < 6) {
        sf = 6;
    } else if (sf > 12) {
        sf = 12;
    }

    if (sf == 6) {
        writeReg(spidev, REG_DETECTION_OPTIMIZE, 0xc5);
        writeReg(spidev, REG_DETECTION_THRESHOLD, 0x0c);
    } else {
        writeReg(spidev, REG_DETECTION_OPTIMIZE, 0xc3);
        writeReg(spidev, REG_DETECTION_THRESHOLD, 0x0a);
    }

    writeReg(spidev, REG_MODEM_CONFIG2, (readReg(spidev, REG_MODEM_CONFIG2) & 0x0f) | ((sf << 4) & 0xf0));
    //MSG_LOG(DEBUG_SPI, "SPI", "SF=0x%02x\n", sf, readReg(spidev, REG_MODEM_CONFIG2));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setsbw(uint8_t spidev, long sbw)
{
    int bw;

    if (sbw <= 7.8E3) {
        bw = 0;
    } else if (sbw <= 10.4E3) {
        bw = 1;
    } else if (sbw <= 15.6E3) {
        bw = 2;
    } else if (sbw <= 20.8E3) {
        bw = 3;
    } else if (sbw <= 31.25E3) {
        bw = 4;
    } else if (sbw <= 41.7E3) {
        bw = 5;
    } else if (sbw <= 62.5E3) {
        bw = 6;
    } else if (sbw <= 125E3) {
        bw = 7;
    } else if (sbw <= 250E3) {
        bw = 8;
    } else /*if (sbw <= 250E3)*/ {
        bw = 9;
    }

    writeReg(spidev, REG_MODEM_CONFIG, (readReg(spidev, REG_MODEM_CONFIG) & 0x0f) | (bw << 4));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setcr(uint8_t spidev, int denominator)
{
    if (denominator < 5) {
        denominator = 5;
    } else if (denominator > 8) {
        denominator = 8;
    }

    int cr = denominator - 4;

    writeReg(spidev, REG_MODEM_CONFIG, (readReg(spidev, REG_MODEM_CONFIG) & 0xf1) | (cr << 1));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setprlen(uint8_t spidev, long length)
{
    writeReg(spidev, REG_PREAMBLE_MSB, (uint8_t)(length >> 8));
    writeReg(spidev, REG_PREAMBLE_LSB, (uint8_t)(length >> 0));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setsyncword(uint8_t spidev, int sw)
{
    writeReg(spidev, REG_SYNC_WORD, sw);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void crccheck(uint8_t spidev, uint8_t nocrc)
{
    if (nocrc)
        writeReg(spidev, REG_MODEM_CONFIG2, readReg(spidev, REG_MODEM_CONFIG2) & 0xfb);
    else
        writeReg(spidev, REG_MODEM_CONFIG2, readReg(spidev, REG_MODEM_CONFIG2) | 0x04);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


/* SPI initialization and configuration */
int lgw_spi_open(char *spi_path) {
    int a = 0, b = 0;
    int i, spidev = 0;

    /* open SPI device */
    spidev = open(spi_path, O_RDWR);
    if (spidev < 0) {
        fprintf(stderr, "ERROR~ failed to open SPI device %s\n", spi_path);
        return -1;
    }

    /* setting SPI mode to 'mode 0' */
    i = SPI_MODE_0;
    a = ioctl(spidev, SPI_IOC_WR_MODE, &i);
    b = ioctl(spidev, SPI_IOC_RD_MODE, &i);
    if ((a < 0) || (b < 0)) {
        fprintf(stderr, "ERROR~(%s): SPI PORT FAIL TO SET IN MODE 0\n", spi_path);
        close(spidev);
        return -1;
    }

    /* setting SPI max clk (in Hz) */
    i = SPI_SPEED;
    a = ioctl(spidev, SPI_IOC_WR_MAX_SPEED_HZ, &i);
    b = ioctl(spidev, SPI_IOC_RD_MAX_SPEED_HZ, &i);
    if ((a < 0) || (b < 0)) {
        fprintf(stderr, "ERROR~(%s): SPI PORT FAIL TO SET MAX SPEED\n", spi_path);
        close(spidev);
        return -1;
    }

    /* setting SPI to MSB first */
    i = 0;
    a = ioctl(spidev, SPI_IOC_WR_LSB_FIRST, &i);
    b = ioctl(spidev, SPI_IOC_RD_LSB_FIRST, &i);
    if ((a < 0) || (b < 0)) {
        fprintf(stderr, "ERROR~(%s): SPI PORT FAIL TO SET MSB FIRST\n", spi_path);
        close(spidev);
        return -1;
    }

    /* setting SPI to 8 bits per word */
    i = 0;
    a = ioctl(spidev, SPI_IOC_WR_BITS_PER_WORD, &i);
    b = ioctl(spidev, SPI_IOC_RD_BITS_PER_WORD, &i);
    if ((a < 0) || (b < 0)) {
        fprintf(stderr, "ERROR~(%s): SPI PORT FAIL TO SET 8 BITS-PER-WORD\n", spi_path);
        close(spidev);
        return -1;
    }

    //fprintf(stderr, "Note(%s): SPI port opened and configured ok\n", spi_path);
    return spidev;
}



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Lora configure : Freq, SF, BW
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

bool get_radio_version(radiodev *dev)
{
    digitalWrite(dev->rst, LOW);
    sleep(1);
    digitalWrite(dev->rst, HIGH);
    sleep(1);

    opmode(dev->spiport, OPMODE_SLEEP);
    uint8_t version = readReg(dev->spiport, REG_VERSION);

    if (version == 0x12) {
        fprintf(stderr, "INFO~ %s: SX1276 detected, starting.\n", dev->desc);
        return true;
    } else {
        fprintf(stderr, "ERROR~ %s: Unrecognized transceiver.\n", dev->desc);
        return false;
    }

}

void setup_channel(radiodev *dev)
{
    MSG_LOG(DEBUG_INFO, "INFO~ Setup %s Channel: freq = %ld, sf = %d, syncwd=0x%02x, prlen=%d, cr=%d/5, spi=%d\n", \
            dev->desc, dev->freq, dev->sf, dev->syncword, dev->prlen, dev->cr - 1, dev->spiport);
    opmode(dev->spiport, OPMODE_SLEEP);
    opmodeLora(dev->spiport);
    ASSERT((readReg(dev->spiport, REG_OPMODE) & OPMODE_LORA) != 0);

    /* setup lora */
    setfreq(dev->spiport, dev->freq);
    setsf(dev->spiport, dev->sf);
    setsbw(dev->spiport, dev->bw);
    setcr(dev->spiport, dev->cr);
    setprlen(dev->spiport, dev->prlen);
    setsyncword(dev->spiport, dev->syncword);

    /* CRC check */
    crccheck(dev->spiport, dev->nocrc);

    // Boost on , 150% LNA current
    writeReg(dev->spiport, REG_LNA, LNA_MAX_GAIN);

    // Auto AGC Low datarate optiomize
    if (dev->sf < 11)
        writeReg(dev->spiport, REG_MODEM_CONFIG3, SX1276_MC3_AGCAUTO);
    else
        writeReg(dev->spiport, REG_MODEM_CONFIG3, SX1276_MC3_LOW_DATA_RATE_OPTIMIZE);

    //500kHz RX optimization
    writeReg(dev->spiport, 0x36, 0x02);
    writeReg(dev->spiport, 0x3a, 0x64);

    // configure output power,RFO pin Output power is limited to +14db
    //writeReg(dev->spiport, REG_PACONFIG, 0x8F);
    //writeReg(dev->spiport, REG_PADAC, readReg(dev->spiport, REG_PADAC)|0x07);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void rxlora(radiodev *dev, uint8_t rxmode)
{

    ASSERT((readReg(dev->spiport, REG_OPMODE) & OPMODE_LORA) != 0);

    /* enter standby mode (required for FIFO loading)) */
    opmode(dev->spiport, OPMODE_STANDBY);

    /* use inverted I/Q signal (prevent mote-to-mote communication) */
    
    if (dev->invertio) {
        writeReg(dev->spiport, REG_INVERTIQ, (readReg(dev->spiport, REG_INVERTIQ) & INVERTIQ_RX_MASK & INVERTIQ_TX_MASK) | INVERTIQ_RX_ON | INVERTIQ_TX_OFF);
        writeReg(dev->spiport, REG_INVERTIQ2, INVERTIQ2_ON);
    } else {
        writeReg(dev->spiport, REG_INVERTIQ, (readReg(dev->spiport, REG_INVERTIQ) & INVERTIQ_RX_MASK & INVERTIQ_TX_MASK) | INVERTIQ_RX_OFF | INVERTIQ_TX_OFF);
        writeReg(dev->spiport, REG_INVERTIQ2, INVERTIQ2_OFF);
    }


    if (rxmode == RXMODE_RSSI) { // use fixed settings for rssi scan
        writeReg(dev->spiport, REG_MODEM_CONFIG, RXLORA_RXMODE_RSSI_REG_MODEM_CONFIG1);
        writeReg(dev->spiport, REG_MODEM_CONFIG2, RXLORA_RXMODE_RSSI_REG_MODEM_CONFIG2);
    }

    writeReg(dev->spiport, REG_MAX_PAYLOAD_LENGTH, 0x80);
    //writeReg(dev->spiport, REG_PAYLOAD_LENGTH, PAYLOAD_LENGTH);
    writeReg(dev->spiport, REG_HOP_PERIOD, 0xFF);
    writeReg(dev->spiport, REG_FIFO_RX_BASE_AD, 0x00);
    writeReg(dev->spiport, REG_FIFO_ADDR_PTR, 0x00);

    // configure DIO mapping DIO0=RxDone DIO1=RxTout DIO2=NOP
    writeReg(dev->spiport, REG_DIO_MAPPING_1, MAP_DIO0_LORA_RXDONE|MAP_DIO1_LORA_RXTOUT|MAP_DIO2_LORA_NOP);

    // clear all radio IRQ flags
    writeReg(dev->spiport, REG_IRQ_FLAGS, 0xFF);
    // enable required radio IRQs
    writeReg(dev->spiport, REG_IRQ_FLAGS_MASK, ~rxlorairqmask[rxmode]);

    //setsyncword(dev->spiport, LORA_MAC_PREAMBLE);  //LoraWan syncword

    // now instruct the radio to receive
    if (rxmode == RXMODE_SINGLE) { // single rx
        //printf("start rx_single\n");
        opmode(dev->spiport, OPMODE_RX_SINGLE);
        //writeReg(dev->spiport, REG_OPMODE, OPMODE_RX_SINGLE);
    } else { // continous rx (scan or rssi)
        opmode(dev->spiport, OPMODE_RX);
    }

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

bool received(uint8_t spidev, struct pkt_rx_s *pkt_rx) {

    int i, rssicorr;

    int irqflags = readReg(spidev, REG_IRQ_FLAGS);

    // clean all IRQ
    writeReg(spidev, REG_IRQ_FLAGS, 0xFF);

    //printf("Start receive, flags=%d\n", irqflags);

    if ((irqflags & IRQ_LORA_RXDONE_MASK) && (irqflags & IRQ_LORA_CRCERR_MASK) == 0) {

        uint8_t currentAddr = readReg(spidev, REG_FIFO_RX_CURRENT_ADDR);
        uint8_t receivedCount = readReg(spidev, REG_RX_NB_BYTES);

        pkt_rx->size = receivedCount;

        writeReg(spidev, REG_FIFO_ADDR_PTR, currentAddr);

        printf("\nRXTX~ Receive(HEX):");
        for(i = 0; i < receivedCount; i++) {
            pkt_rx->payload[i] = (char)readReg(spidev, REG_FIFO);
            printf("%02x", pkt_rx->payload[i]);
        }
        printf("\n");

        uint8_t value = readReg(spidev, REG_PKT_SNR_VALUE);

        if( value & 0x80 ) // The SNR sign bit is 1
        {
            // Invert and divide by 4
            value = ( ( ~value + 1 ) & 0xFF ) >> 2;
            pkt_rx->snr = -value;
        } else {
            // Divide by 4
            pkt_rx->snr = ( value & 0xFF ) >> 2;
        }
        
        rssicorr = 157;

        pkt_rx->rssi = readReg(spidev, REG_PKTRSSI) - rssicorr;

        pkt_rx->empty = 0;  /* make sure save the received messaeg */

        return true;
    } /* else if (readReg(spidev, REG_OPMODE) != (OPMODE_LORA | OPMODE_RX_SINGLE)) {  //single mode
        writeReg(spidev, REG_FIFO_ADDR_PTR, 0x00);
        rxlora(spidev, RXMODE_SINGLE);
    }*/
    return false;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void txlora(radiodev *dev, struct pkt_tx_s *pkt) {

    int i;

    struct timeval current_unix_time;
    uint32_t time_us;

    opmode(dev->spiport, OPMODE_SLEEP);
    // select LoRa modem (from sleep mode)
    opmodeLora(dev->spiport);

    ASSERT((readReg(dev->spiport, REG_OPMODE) & OPMODE_LORA) != 0);

    setpower(dev->spiport, pkt->rf_power);
    setfreq(dev->spiport, pkt->freq_hz);
    setsf(dev->spiport, sf_getval(pkt->datarate));
    setsbw(dev->spiport, bw_getval(pkt->bandwidth));
    setcr(dev->spiport, pkt->coderate);
    setprlen(dev->spiport, pkt->preamble);
    setsyncword(dev->spiport, dev->syncword);

    /* CRC check */
    crccheck(dev->spiport, pkt->no_crc);

    // Boost on , 150% LNA current
    writeReg(dev->spiport, REG_LNA, LNA_MAX_GAIN);

    writeReg(dev->spiport, REG_PARAMP, 0x08);

    // Auto AGC
    if (dev->sf < 11)
        writeReg(dev->spiport, REG_MODEM_CONFIG3, SX1276_MC3_AGCAUTO);
    else
        writeReg(dev->spiport, REG_MODEM_CONFIG3, SX1276_MC3_LOW_DATA_RATE_OPTIMIZE);

    // configure output power,RFO pin Output power is limited to +14db
    writeReg(dev->spiport, REG_PACONFIG, (uint8_t)(0x80|(15&0xf)));

    if (pkt->invert_pol) {
        writeReg(dev->spiport, REG_INVERTIQ, (readReg(dev->spiport, REG_INVERTIQ) & INVERTIQ_RX_MASK & INVERTIQ_TX_MASK) | INVERTIQ_RX_OFF | INVERTIQ_TX_ON);
        writeReg(dev->spiport, REG_INVERTIQ2, INVERTIQ2_ON);
    } else {
        writeReg(dev->spiport, REG_INVERTIQ, (readReg(dev->spiport, REG_INVERTIQ) & INVERTIQ_RX_MASK & INVERTIQ_TX_MASK) | INVERTIQ_RX_OFF | INVERTIQ_TX_OFF);
        writeReg(dev->spiport, REG_INVERTIQ2, INVERTIQ2_OFF);
    }

    // enter standby mode (required for FIFO loading))
    opmode(dev->spiport, OPMODE_STANDBY);

    // set the IRQ mapping DIO0=TxDone DIO1=NOP DIO2=NOP
    writeReg(dev->spiport, REG_DIO_MAPPING_1, MAP_DIO0_LORA_TXDONE|MAP_DIO1_LORA_NOP|MAP_DIO2_LORA_NOP);
    // clear all radio IRQ flags
    writeReg(dev->spiport, REG_IRQ_FLAGS, 0xFF);
    // mask all IRQs but TxDone
    writeReg(dev->spiport, REG_IRQ_FLAGS_MASK, ~IRQ_LORA_TXDONE_MASK);

    // initialize the payload size and address pointers
    writeReg(dev->spiport, REG_FIFO_TX_BASE_AD, 0x00); writeReg(dev->spiport, REG_FIFO_ADDR_PTR, 0x00);

    for (i = 0; i < pkt->size; i++) { 
        writeReg(dev->spiport, REG_FIFO, pkt->payload[i]);
    }

    writeReg(dev->spiport, REG_PAYLOAD_LENGTH, pkt->size);

    gettimeofday(&current_unix_time, NULL);
    time_us = current_unix_time.tv_sec * 1000000UL + current_unix_time.tv_usec;

    time_us = pkt->count_us - 1495/*TX_START_DELAY*/ - time_us;

    if (pkt->tx_mode != IMMEDIATE && time_us > 0 && time_us < 30000/*TX_JIT DELAY*/)
        wait_us(time_us);

    // now we actually start the transmission
    opmode(dev->spiport, OPMODE_TX);

    // wait for TX done
    while(digitalRead(dev->dio[0]) == 0);

    MSG_LOG(DEBUG_INFO, "\nINFO~ Transmit at SF%iBW%ld on %.6lf.\n", sf_getval(pkt->datarate), bw_getval(pkt->bandwidth)/1000, (double)(pkt->freq_hz)/1000000);

    // mask all IRQs
    writeReg(dev->spiport, REG_IRQ_FLAGS_MASK, 0xFF);

    // clear all radio IRQ flags
    writeReg(dev->spiport, REG_IRQ_FLAGS, 0xFF);

    // go from stanby to sleep
    opmode(dev->spiport, OPMODE_SLEEP);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void single_tx(radiodev *dev, uint8_t *payload, int size) {

    ASSERT((readReg(dev->spiport, REG_OPMODE) & OPMODE_LORA) != 0);

    // enter standby mode (required for FIFO loading))
    opmode(dev->spiport, OPMODE_STANDBY);

    if (dev->invertio) {
        writeReg(dev->spiport, REG_INVERTIQ, (readReg(dev->spiport, REG_INVERTIQ) & INVERTIQ_RX_MASK & INVERTIQ_TX_MASK) | INVERTIQ_RX_OFF | INVERTIQ_TX_ON);
        writeReg(dev->spiport, REG_INVERTIQ2, INVERTIQ2_ON);
    } else {
        writeReg(dev->spiport, REG_INVERTIQ, (readReg(dev->spiport, REG_INVERTIQ) & INVERTIQ_RX_MASK & INVERTIQ_TX_MASK) | INVERTIQ_RX_OFF | INVERTIQ_TX_OFF);
        writeReg(dev->spiport, REG_INVERTIQ2, INVERTIQ2_OFF);
    }

    setsyncword(dev->spiport, dev->syncword);

    setpower(dev->spiport, dev->power);

    // set the IRQ mapping DIO0=TxDone DIO1=NOP DIO2=NOP
    writeReg(dev->spiport, REG_DIO_MAPPING_1, MAP_DIO0_LORA_TXDONE|MAP_DIO1_LORA_NOP|MAP_DIO2_LORA_NOP);
    // clear all radio IRQ flags
    writeReg(dev->spiport, REG_IRQ_FLAGS, 0xFF);
    // mask all IRQs but TxDone
    writeReg(dev->spiport, REG_IRQ_FLAGS_MASK, ~IRQ_LORA_TXDONE_MASK);

    // initialize the payload size and address pointers
    writeReg(dev->spiport, REG_FIFO_TX_BASE_AD, 0x00); writeReg(dev->spiport, REG_FIFO_ADDR_PTR, 0x00);

    // write buffer to the radio FIFO
    int i;

    for (i = 0; i < size; i++) { 
        writeReg(dev->spiport, REG_FIFO, payload[i]);
    }

    writeReg(dev->spiport, REG_PAYLOAD_LENGTH, size);

    // now we actually start the transmission
    opmode(dev->spiport, OPMODE_TX);

    // wait for TX done
    while(digitalRead(dev->dio[0]) == 0);

    MSG_LOG(DEBUG_INFO, "\nINFO~Transmit at SF%iBW%ld on %.6lf.\n", dev->sf, (dev->bw)/1000, (double)(dev->freq)/1000000);

    // mask all IRQs
    writeReg(dev->spiport, REG_IRQ_FLAGS_MASK, 0xFF);

    // clear all radio IRQ flags
    writeReg(dev->spiport, REG_IRQ_FLAGS, 0xFF);

    // go from stanby to sleep
    opmode(dev->spiport, OPMODE_SLEEP);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t bw_getval(int x) {
    switch (x) {
        case BW_500KHZ: return 500000;
        case BW_250KHZ: return 250000;
        case BW_125KHZ: return 125000;
        case BW_62K5HZ: return 62500;
        case BW_31K2HZ: return 31200;
        case BW_15K6HZ: return 15600;
        case BW_7K8HZ : return 7800;
        default: return -1;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t sf_getval(int x) {
    switch (x) {
        case DR_LORA_SF7: return 7;
        case DR_LORA_SF8: return 8;
        case DR_LORA_SF9: return 9;
        case DR_LORA_SF10: return 10;
        case DR_LORA_SF11: return 11;
        case DR_LORA_SF12: return 12;
        default: return -1;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t sf_toval(int x) {
    switch (x) {
        case 7: return DR_LORA_SF7; 
        case 8: return DR_LORA_SF8; 
        case 9: return DR_LORA_SF9;
        case 10: return DR_LORA_SF10;
        case 11: return DR_LORA_SF11;
        case 12: return DR_LORA_SF12;
        default: return -1;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t bw_toval(int x) {
    switch (x) {
        case 500000: return BW_500KHZ;
        case 250000: return BW_250KHZ;
        case 125000: return BW_125KHZ;
        case 62500: return BW_62K5HZ;
        case 31200: return BW_31K2HZ;
        case 15600: return BW_15K6HZ;
        case 7800: return BW_7K8HZ;
        default: return -1;
    }
}
