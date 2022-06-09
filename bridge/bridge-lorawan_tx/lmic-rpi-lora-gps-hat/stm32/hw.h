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



#include "oslmic.h"
#include "stm32l1xx.h"

//////////////////////////////////////////////////////////////////////
// GPIO
//////////////////////////////////////////////////////////////////////

// GPIO by port number (A=0, B=1, ..)
#define GPIOx(no) ((GPIO_TypeDef*) (GPIOA_BASE + (no)*(GPIOB_BASE-GPIOA_BASE)))

// GPIOCFG macros
#define GPIOCFG_AF_MASK        0x000F
#define GPIO_AF_I2C1        0x04

#define GPIOCFG_MODE_SHIFT      4
#define GPIOCFG_MODE_MASK      (3<<GPIOCFG_MODE_SHIFT)
#define GPIOCFG_MODE_INP       (0<<GPIOCFG_MODE_SHIFT)
#define GPIOCFG_MODE_OUT       (1<<GPIOCFG_MODE_SHIFT)
#define GPIOCFG_MODE_ALT       (2<<GPIOCFG_MODE_SHIFT)
#define GPIOCFG_MODE_ANA       (3<<GPIOCFG_MODE_SHIFT)
#define GPIOCFG_OSPEED_SHIFT    6
#define GPIOCFG_OSPEED_MASK    (3<<GPIOCFG_OSPEED_SHIFT)
#define GPIOCFG_OSPEED_400kHz  (0<<GPIOCFG_OSPEED_SHIFT)
#define GPIOCFG_OSPEED_2MHz    (1<<GPIOCFG_OSPEED_SHIFT)
#define GPIOCFG_OSPEED_10MHz   (2<<GPIOCFG_OSPEED_SHIFT)
#define GPIOCFG_OSPEED_40MHz   (3<<GPIOCFG_OSPEED_SHIFT)
#define GPIOCFG_OTYPE_SHIFT     8
#define GPIOCFG_OTYPE_MASK     (1<<GPIOCFG_OTYPE_SHIFT)
#define GPIOCFG_OTYPE_PUPD     (0<<GPIOCFG_OTYPE_SHIFT)
#define GPIOCFG_OTYPE_OPEN     (1<<GPIOCFG_OTYPE_SHIFT)
#define GPIOCFG_PUPD_SHIFT      9
#define GPIOCFG_PUPD_MASK      (3<<GPIOCFG_PUPD_SHIFT)
#define GPIOCFG_PUPD_NONE      (0<<GPIOCFG_PUPD_SHIFT)
#define GPIOCFG_PUPD_PUP       (1<<GPIOCFG_PUPD_SHIFT)
#define GPIOCFG_PUPD_PDN       (2<<GPIOCFG_PUPD_SHIFT)
#define GPIOCFG_PUPD_RFU       (3<<GPIOCFG_PUPD_SHIFT)

// IRQ triggers (same values as in Moterunner!)
#define GPIO_IRQ_MASK          0x38
#define GPIO_IRQ_FALLING       0x20
#define GPIO_IRQ_RISING        0x28
#define GPIO_IRQ_CHANGE        0x30

// configure operation mode of GPIO pin
void hw_cfg_pin (GPIO_TypeDef* gpioport, u1_t pin, u2_t gpiocfg);

// set state of GPIO output pin
void hw_set_pin (GPIO_TypeDef* gpioport, u1_t pin, u1_t state);

// configure given line as external interrupt source (EXTI handler)
void hw_cfg_extirq (u1_t portidx, u1_t pin, u1_t irqcfg);


//////////////////////////////////////////////////////////////////////
// EEPROM
//////////////////////////////////////////////////////////////////////

// Unique device ID registers (96 bits)
#define UNIQUE_ID_BASE 0x1FF80050
#define UNIQUE_ID0 (UNIQUE_ID_BASE+0x00)
#define UNIQUE_ID1 (UNIQUE_ID_BASE+0x04)
#define UNIQUE_ID2 (UNIQUE_ID_BASE+0x14)

// EEPROM 0x08080000-0x08080FFF 4096 bytes
#define EEPROM_BASE 0x08080000

// write 32-bit word to EEPROM
void eeprom_write (u4_t* addr, u4_t val);

// copy bytes to EEPROM (aligned, multiple of 4)
void eeprom_copy (void* dst, const void* src, u2_t len);


//////////////////////////////////////////////////////////////////////
// ADC
//////////////////////////////////////////////////////////////////////

u2_t adc_read (u1_t chnl);


//////////////////////////////////////////////////////////////////////
// CRC engine (32bit aligned words only!)
//////////////////////////////////////////////////////////////////////

void crc32_init (void);
unsigned int crc32 (void* ptr, int nwords);
void crc32_shutdown (void);


//////////////////////////////////////////////////////////////////////
// I2C
//////////////////////////////////////////////////////////////////////

// initialize i2c-bus
u1_t i2c_init (void);

// transfer data to and from i2c-device
s1_t i2c_xfer (u1_t addr, u1_t* data, u1_t wlen, u1_t rlen);


//////////////////////////////////////////////////////////////////////
// SPI
//////////////////////////////////////////////////////////////////////

#define SPI_MODE_CPHA  0x01
#define SPI_MODE_CPOL  0x02

// initialize SPI (modes 0-3, bit0=clock phase, bit1=clock polarity)
void spi_init (u1_t mode);

// transfer byte to and from SPI (no chip-select)
u1_t spi_xfer (u1_t out);






//////////////////////////////////////////////////////////////////////
// FLASH
//////////////////////////////////////////////////////////////////////
#ifdef CFG_flash

typedef u2_t const * pref2u2_t;
typedef u1_t const * pref2u1_t;
typedef pref2u1_t pref_t;


pref2u1_t hw_flash_init();
void hal_erase_p (pref2u1_t ppdst, u2_t len, u1_t itemsize);
void hal_copy_x2p (pref2u1_t ppdst, u2_t len, xref2u1_t xpsrc);
void hal_wrp_u1 (pref2u1_t ppdst, u1_t value);
void hal_wrp_u2 (pref2u2_t ppdst, u2_t value);


#endif /* CFG_flash */

