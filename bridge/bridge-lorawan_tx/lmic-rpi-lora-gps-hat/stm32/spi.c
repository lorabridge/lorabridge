/*
 * Copyright (c) 2014-2016 IBM Corporation.
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "hw.h"

//////////////////////////////////////////////////////////////////////
// SPI 2
//////////////////////////////////////////////////////////////////////

//                    // NSS:  PB12

#define SCK_PORT   1  // SCK:  PB13
#define SCK_PIN    13
#define MISO_PORT  1  // MISO: PB14
#define MISO_PIN   14
#define MOSI_PORT  1  // MOSI: PB15
#define MOSI_PIN   15

#define GPIO_AF_SPI2        0x05

void spi_init (u1_t mode) {
    // enable clocks
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN; // GPIO ports B
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; // SPI interface 2

    // reset/stop
    SPI2->CR1 = 0; // clear SPE

    // use alternate function SPI2 (SCK, MISO, MOSI)
    hw_cfg_pin(GPIOx(SCK_PORT),  SCK_PIN,  GPIOCFG_MODE_ALT | GPIOCFG_OSPEED_40MHz | GPIO_AF_SPI2 | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PDN);
    hw_cfg_pin(GPIOx(MISO_PORT), MISO_PIN, GPIOCFG_MODE_ALT | GPIOCFG_OSPEED_40MHz | GPIO_AF_SPI2 | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PDN);
    hw_cfg_pin(GPIOx(MOSI_PORT), MOSI_PIN, GPIOCFG_MODE_ALT | GPIOCFG_OSPEED_40MHz | GPIO_AF_SPI2 | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PDN);
    
    // configure and activate the SPI (master, internal slave select, software slave mgmt)
    // (use default mode: 8-bit, 2-wire, no crc, MSBF, PCLK/2, CPOL0, CPHA0)
    SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_SPE | (mode & 0x03);
}

// perform SPI transaction
u1_t spi_xfer (u1_t out) {
    SPI2->DR = out;
    while( (SPI2->SR & SPI_SR_RXNE ) == 0);
    return SPI2->DR; // in
}
