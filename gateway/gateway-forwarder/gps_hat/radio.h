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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef _RADIO_H
#define _RADIO_H

#define ASSERT(cond) if(!(cond)) {printf("%s:%d\n", __FILE__, __LINE__); exit(1);}

/* values available for the 'tx_mode' parameter */
#define IMMEDIATE       0
#define TIMESTAMPED     1
#define ON_GPS          2

#define MSG(args...)	        printf(args) /* message that is destined to the user */
#define MSG_DEBUG(FLAG, fmt, ...)                                                               \
    do {                                                                                       \
        if (FLAG)                                                                                 \
            fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
    } while (0)

#define MSG_LOG(LEVEL, fmt, ...)                                                               \
    do {                                                                                      \
        if (LEVEL)                                                                             \
            fprintf(stdout, fmt, ##__VA_ARGS__);                                                    \
    } while (0)

// ####################################################################
// Registers Mapping
// ####################################################################

#define REG_FIFO                    0x00
#define REG_OPMODE                  0x01
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_AD         0x0E
#define REG_FIFO_RX_BASE_AD         0x0F
#define REG_RX_NB_BYTES             0x13
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_MODEM_CONFIG            0x1D
#define REG_MODEM_CONFIG2           0x1E
#define REG_MODEM_CONFIG3           0x26
#define REG_SYMB_TIMEOUT_LSB        0x1F
#define REG_PKT_SNR_VALUE           0x19
#define REG_PAYLOAD_LENGTH          0x22
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_MAX_PAYLOAD_LENGTH      0x23
#define REG_HOP_PERIOD              0x24
#define REG_SYNC_WORD               0x39
#define REG_VERSION                 0x42
#define REG_PADAC                   0x4D
#define REG_PKTRSSI                 0x1A
#define REG_RSSI                    0x1B 

#define  REG_DETECTION_OPTIMIZE     0x31
#define  REG_DETECTION_THRESHOLD    0x37

#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21

// preamble for lora networks (nibbles swapped)
#define LORA_MAC_PREAMBLE           0x34

#define PAYLOAD_LENGTH              0x40

// LOW NOISE AMPLIFIER
#define REG_LNA                     0x0C
#define LNA_MAX_GAIN                0x23
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN                0x20

// ----------------------------------------
// Constants for radio registers
#define OPMODE_LORA      0x80
#define OPMODE_MASK      0x07
#define OPMODE_SLEEP     0x00
#define OPMODE_STANDBY   0x01
#define OPMODE_FSTX      0x02
#define OPMODE_TX        0x03
#define OPMODE_FSRX      0x04
#define OPMODE_RX        0x05
#define OPMODE_RX_SINGLE 0x06
#define OPMODE_CAD       0x07

// ----------------------------------------
// Bits masking the corresponding IRQs from the radio
#define IRQ_LORA_RXTOUT_MASK 0x80
#define IRQ_LORA_RXDONE_MASK 0x40
#define IRQ_LORA_CRCERR_MASK 0x20
#define IRQ_LORA_HEADER_MASK 0x10
#define IRQ_LORA_TXDONE_MASK 0x08
#define IRQ_LORA_CDDONE_MASK 0x04
#define IRQ_LORA_FHSSCH_MASK 0x02
#define IRQ_LORA_CDDETD_MASK 0x01

// ----------------------------------------
// DIO function mappings                D0D1D2D3
#define MAP_DIO0_LORA_RXDONE   0x00  // 00------
#define MAP_DIO0_LORA_TXDONE   0x40  // 01------
#define MAP_DIO1_LORA_RXTOUT   0x00  // --00----
#define MAP_DIO1_LORA_NOP      0x30  // --11----
#define MAP_DIO2_LORA_NOP      0xC0  // ----11--

// sx1276 RegModemConfig2
#define SX1276_MC2_RX_PAYLOAD_CRCON        0x04

// sx1276 RegModemConfig3
#define SX1276_MC3_LOW_DATA_RATE_OPTIMIZE  0x08
#define SX1276_MC3_AGCAUTO                 0x04

// FRF
#define        REG_FRF_MSB              0x06
#define        REG_FRF_MID              0x07
#define        REG_FRF_LSB              0x08

#define        REG_PACONFIG              0x09 // common
#define        REG_PARAMP                0x0A // common

#define        REG_INVERTIQ              0x33
#define        REG_INVERTIQ2             0x3B

// CONF REG
#define RXLORA_RXMODE_RSSI_REG_MODEM_CONFIG1 0x0A
#define RXLORA_RXMODE_RSSI_REG_MODEM_CONFIG2 0x70

#define MAXLINE 256


/*!                                                                                                     
 * RegInvertIQ                                                                                
*/                                                                                                     
#define INVERTIQ_RX_MASK                       0xBF                                                
#define INVERTIQ_RX_OFF                        0x00                                                
#define INVERTIQ_RX_ON                         0x40                                                
#define INVERTIQ_TX_MASK                       0xFE                                                
#define INVERTIQ_TX_OFF                        0x01                                                
#define INVERTIQ_TX_ON                         0x00                                                
                                                                                                        
/*!                                                                                                     
 * RegDetectionThreshold                                                                                
*/                                                                                                     
#define DETECTIONTHRESH_SF7_TO_SF12            0x0A // Default
#define DETECTIONTHRESH_SF6                    0x0C

/*!
 * RegInvertIQ2
*/
#define INVERTIQ2_ON                           0x19
#define INVERTIQ2_OFF                          0x1D

/* values available for the 'bandwidth' parameters (LoRa & FSK) */
/* NOTE: directly encode FSK RX bandwidth, do not change */
#define BW_UNDEFINED    0
#define BW_500KHZ       0x01
#define BW_250KHZ       0x02
#define BW_125KHZ       0x03
#define BW_62K5HZ       0x04
#define BW_31K2HZ       0x05
#define BW_15K6HZ       0x06
#define BW_7K8HZ        0x07

/* values available for the 'datarate' parameters */
/* NOTE: LoRa values used directly to code SF bitmask in 'multi' modem, do not change */
#define DR_UNDEFINED    0
#define DR_LORA_SF7     0x02
#define DR_LORA_SF8     0x04
#define DR_LORA_SF9     0x08
#define DR_LORA_SF10    0x10
#define DR_LORA_SF11    0x20
#define DR_LORA_SF12    0x40
#define DR_LORA_MULTI   0x7E

/* values available for the 'coderate' parameters (LoRa only) */
/* NOTE: arbitrary values */
#define CR_UNDEFINED    0
#define CR_LORA_4_5     5
#define CR_LORA_4_6     6
#define CR_LORA_4_7     7
#define CR_LORA_4_8     8

// ##################################################################
// rxmode 
// ##################################################################
//
enum { RXMODE_SINGLE, RXMODE_SCAN, RXMODE_RSSI };


/*******************************************************************************
 * radio configure
 *******************************************************************************/

enum sf_t { SF7=7, SF8, SF9, SF10, SF11, SF12 };

typedef struct {
    uint8_t nss;
    uint8_t rst;
    uint8_t dio[3];
    uint8_t  spiport;
    uint32_t freq;
    uint32_t bw;
    uint8_t sf;
    uint8_t cr;
    uint8_t nocrc;
    uint8_t prlen;
    uint8_t syncword;
    uint8_t invertio;
    uint8_t power;
    char desc[8];
}radiodev; 

/**
 * @struct lgw_pkt_tx_s
 * @brief Structure containing the configuration of a packet to send and a pointer to the payload
 * */  
struct pkt_tx_s {
    uint32_t    freq_hz;        /*!> center frequency of TX */
    uint8_t     tx_mode;        /*!> select on what event/time the TX is triggered */
    uint32_t    count_us;       /*!> timestamp or delay in microseconds for TX trigger */
    uint8_t     bandwidth;      /*!> modulation bandwidth (LoRa only) */
    uint8_t     rf_power;       /*!> TX power, in dBm */
    uint32_t    datarate;       /*!> TX datarate (baudrate for FSK, SF for LoRa) */
    uint8_t     coderate;       /*!> error-correcting code of the packet (LoRa only) */
    bool        invert_pol;     /*!> invert signal polarity, for orthogonal downlinks (LoRa only) */
    uint16_t    preamble;       /*!> set the preamble length, 0 for default */
    bool        no_crc;         /*!> if true, do not send a CRC in the packet */
    bool        no_header;      /*!> if true, enable implicit header mode (LoRa), fixed length (FSK) */
    uint16_t    size;           /*!> payload size in bytes */
    uint8_t     payload[256];   /*!> buffer containing the payload */
};

/**
 @struct pkt_rx_s
 @brief Structure containing the metadata of a packet that was received and a pointer to the payload
*/

struct pkt_rx_s {
    uint8_t     empty;        /*!> empty label */
    float       snr;          /*!> average packet SNR, in dB (LoRa only) */
    float       rssi;         /*!> average packet RSSI in dB */
    uint16_t    crc;          /*!> CRC that was received in the payload */
    uint16_t    size;         /*!> payload size in bytes */ 
    uint8_t     payload[256]; /*!> buffer containing the payload */
};

#define QUEUESIZE 8         /*!> size of queue */


/*******************************************************************************
 * GPIO/SPI configure
********************************************************************************/

#define LOW 		0
#define HIGH 		1

#define GPIO_OUT        0
#define GPIO_IN         1

#define READ_ACCESS     0x00
#define WRITE_ACCESS    0x80
#define SPI_SPEED       8000000
#define SPI_DEV_RX    "/dev/spidev0.0"
#define SPI_DEV_TX    "/dev/spidev0.1"

/* log level */
extern int DEBUG_PKT_FWD;
extern int DEBUG_JIT;
extern int DEBUG_JIT_ERROR;
extern int DEBUG_TIMERSYNC;
extern int DEBUG_BEACON;
extern int DEBUG_INFO;
extern int DEBUG_WARNING;
extern int DEBUG_ERROR;
extern int DEBUG_GPS;
extern int DEBUG_SPI;
extern int DEBUG_UCI;


void wait_ms(unsigned long a); 

void wait_us(unsigned long a); 

/*
 * Read the state of the port. The port can be input
 * or output.
 * @gpio the GPIO pin to read from.
 * @return GPIO_HIGH if the pin is HIGH, GPIO_LOW if the pin is low. Output
 * is -1 when an error occurred.
 */
int digitalRead(int);

void diditalWrite(int, int);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* SPI initialization and configuration */
int lgw_spi_open(char *); 

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Lora configure : Freq, SF, BW
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setpower(uint8_t, uint8_t);

void setfreq(uint8_t, long);

void setsf(uint8_t, int);

void setsbw(uint8_t, long);

void setcr(uint8_t, int);

void setprlen(uint8_t, long);

void setsyncword(uint8_t, int);

void crccheck(uint8_t, uint8_t);

bool get_radio_version(radiodev *);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void setup_channel(radiodev *);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void rxlora(radiodev *, uint8_t);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

bool received(uint8_t, struct pkt_rx_s *); 

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void txlora(radiodev *, struct pkt_tx_s *); 

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void single_tx(radiodev *, uint8_t *, int); 

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t bw_getval(int); 

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t bw_toval(int); 

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t sf_getval(int);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t sf_toval(int);
    
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#endif   //defined _RADIO_H
