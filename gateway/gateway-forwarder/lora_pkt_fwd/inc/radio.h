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

#ifndef _RADIO_H
#define _RADIO_H

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
#define REG_PADAC                   0x4D // common
#define REG_PKTRSSI                 0x1A
#define REG_RSSI                    0x1B 

#define  REG_DETECTION_OPTIMIZE     0x31
#define  REG_DETECTION_THRESHOLD    0x37

#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21

#define PAYLOAD_LENGTH              0x40

// preamble for lora networks (nibbles swapped)
#define LORA_MAC_PREAMBLE                  0x34


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

/*!                                                                                                     
 * RegInvertIQ                                                                                
 */                      
#define INVERTIQ_RX_MASK                       0xBF
#define INVERTIQ_RX_OFF                        0x00                                                
#define INVERTIQ_RX_ON                         0x40                                                
#define INVERTIQ_TX_MASK                       0xFE                                                
#define INVERTIQ_TX_OFF                        0x01
#define INVERTIQ_TX_ON                         0x00

#define INVERTIQ2_ON                           0x19
#define INVERTIQ2_OFF                          0x1D

#define MAXLINE 256


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
    uint8_t invertio;
    uint8_t syncword;
    int8_t rf_power;
    char desc[8];
}radiodev; 

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
#define SPI_DEV_RADIO    "/dev/spidev0.1"

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
int spi_open(char *); 

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

void rxlora(int, uint8_t);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

bool received(uint8_t, struct lgw_pkt_rx_s *);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void txlora(radiodev *, struct lgw_pkt_tx_s *);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void single_tx(radiodev *, uint8_t *, int); 

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#endif   //defined _RADIO_H
