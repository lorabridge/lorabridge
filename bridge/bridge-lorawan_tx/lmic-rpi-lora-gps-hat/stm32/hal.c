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

#include "lmic.h"
#include "hw.h"

// -----------------------------------------------------------------------------
// I/O

#ifdef CFG_sx1276mb1_board

#define NSS_PORT           1 // NSS: PB6, sx1276
#define NSS_PIN            6  // sx1276: PB6

#define TX_PORT            2 // TX:  PC1
#define TX_PIN             1

#define RST_PORT           0 // RST: PA0
#define RST_PIN            0

#define DIO0_PORT          0 // DIO0: PA10, sx1276   (line 1 irq handler)
#define DIO0_PIN           10
#define DIO1_PORT          1 // DIO1: PB3, sx1276  (line 10-15 irq handler)
#define DIO1_PIN           3
#define DIO2_PORT          1 // DIO2: PB5, sx1276  (line 10-15 irq handler)
#define DIO2_PIN           5

static const u1_t outputpins[] = { NSS_PORT, NSS_PIN, TX_PORT, TX_PIN  };
static const u1_t inputpins[]  = { DIO0_PORT, DIO0_PIN, DIO1_PORT, DIO1_PIN, DIO2_PORT, DIO2_PIN };

#elif CFG_wimod_board

// output lines
#define NSS_PORT           1 // NSS: PB0, sx1272
#define NSS_PIN            0

#define TX_PORT            0 // TX:  PA4
#define TX_PIN             4
#define RX_PORT            2 // RX:  PC13
#define RX_PIN            13
#define RST_PORT           0 // RST: PA2
#define RST_PIN            2

// input lines
#define DIO0_PORT          1 // DIO0: PB1   (line 1 irq handler)
#define DIO0_PIN           1
#define DIO1_PORT          1 // DIO1: PB10  (line 10-15 irq handler)
#define DIO1_PIN          10
#define DIO2_PORT          1 // DIO2: PB11  (line 10-15 irq handler)
#define DIO2_PIN          11

static const u1_t outputpins[] = { NSS_PORT, NSS_PIN, TX_PORT, TX_PIN, RX_PORT, RX_PIN };
static const u1_t inputpins[]  = { DIO0_PORT, DIO0_PIN, DIO1_PORT, DIO1_PIN, DIO2_PORT, DIO2_PIN };

#else
#error Missing CFG_sx1276mb1_board/CFG_wimod_board!
#endif

// HAL state
static struct {
    int irqlevel;
    u4_t ticks;
} HAL;

// -----------------------------------------------------------------------------
// I/O

static void hal_io_init () {
    // clock enable for GPIO ports A,B,C
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

    // configure output lines and set to low state
    for(u1_t i=0; i<sizeof(outputpins); i+=2) {
        hw_cfg_pin(GPIOx(outputpins[i]), outputpins[i+1], GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
        hw_set_pin(GPIOx(outputpins[i]), outputpins[i+1], 0);
    }

    // configure input lines and register IRQ
    for(u1_t i=0; i<sizeof(inputpins); i+=2) {
        hw_cfg_pin(GPIOx(inputpins[i]), inputpins[i+1], GPIOCFG_MODE_INP | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_OPEN);
        hw_cfg_extirq(inputpins[i], inputpins[i+1], GPIO_IRQ_RISING);
    }
}

// val ==1  => tx 1, rx 0 ; val == 0 => tx 0, rx 1
void hal_pin_rxtx (u1_t val) {
    ASSERT(val == 1 || val == 0);
#ifndef CFG_sx1276mb1_board
    hw_set_pin(GPIOx(RX_PORT), RX_PIN, ~val);
#endif
    hw_set_pin(GPIOx(TX_PORT), TX_PIN, val);
}


// set radio NSS pin to given value
void hal_pin_nss (u1_t val) {
    hw_set_pin(GPIOx(NSS_PORT), NSS_PIN, val);
}

// set radio RST pin to given value (or keep floating!)
void hal_pin_rst (u1_t val) {
    if(val == 0 || val == 1) { // drive pin
        hw_cfg_pin(GPIOx(RST_PORT), RST_PIN, GPIOCFG_MODE_OUT | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PUP);
        hw_set_pin(GPIOx(RST_PORT), RST_PIN, val);
    } else { // keep pin floating
        hw_cfg_pin(GPIOx(RST_PORT), RST_PIN, GPIOCFG_MODE_INP | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_OPEN);
    }
}

extern void radio_irq_handler(u1_t dio);

// generic EXTI IRQ handler for all channels
void EXTI_IRQHandler () {
    // DIO 0
    if((EXTI->PR & (1<<DIO0_PIN)) != 0) { // pending
        EXTI->PR = (1<<DIO0_PIN); // clear irq
        // invoke radio handler (on IRQ!)
        radio_irq_handler(0);
    }
    // DIO 1
    if((EXTI->PR & (1<<DIO1_PIN)) != 0) { // pending
        EXTI->PR = (1<<DIO1_PIN); // clear irq
        // invoke radio handler (on IRQ!)
        radio_irq_handler(1);
    }
    // DIO 2
    if((EXTI->PR & (1<<DIO2_PIN)) != 0) { // pending
        EXTI->PR = (1<<DIO2_PIN); // clear irq
        // invoke radio handler (on IRQ!)
        radio_irq_handler(2);
    }
       
#ifdef CFG_EXTI_IRQ_HANDLER
    // invoke user-defined interrupt handler
    {
        extern void CFG_EXTI_IRQ_HANDLER(void);
        CFG_EXTI_IRQ_HANDLER();
    }
#endif // CFG_EXTI_IRQ_HANDLER
}

#if CFG_lmic_clib
void EXTI0_IRQHandler () {
    EXTI_IRQHandler();
}

void EXTI1_IRQHandler () {
    EXTI_IRQHandler();
}

void EXTI2_IRQHandler () {
    EXTI_IRQHandler();
}

void EXTI3_IRQHandler () {
    EXTI_IRQHandler();
}

void EXTI4_IRQHandler () {
    EXTI_IRQHandler();
}

void EXTI9_5_IRQHandler () {
    EXTI_IRQHandler();
}

void EXTI15_10_IRQHandler () {
    EXTI_IRQHandler();
}
#endif // CFG_lmic_clib

// -----------------------------------------------------------------------------
// SPI

// for sx1272 and 1276

#define SCK_PORT   0 // SCK:  PA5
#define SCK_PIN    5
#define MISO_PORT  0 // MISO: PA6
#define MISO_PIN   6
#define MOSI_PORT  0 // MOSI: PA7
#define MOSI_PIN   7

#define GPIO_AF_SPI1        0x05

static void hal_spi_init () {
    // enable clock for SPI interface 1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // use alternate function SPI1 (SCK, MISO, MOSI)
    hw_cfg_pin(GPIOx(SCK_PORT),  SCK_PIN,  GPIOCFG_MODE_ALT | GPIOCFG_OSPEED_40MHz | GPIO_AF_SPI1 | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PDN);
    hw_cfg_pin(GPIOx(MISO_PORT), MISO_PIN, GPIOCFG_MODE_ALT | GPIOCFG_OSPEED_40MHz | GPIO_AF_SPI1 | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PDN);
    hw_cfg_pin(GPIOx(MOSI_PORT), MOSI_PIN, GPIOCFG_MODE_ALT | GPIOCFG_OSPEED_40MHz | GPIO_AF_SPI1 | GPIOCFG_OTYPE_PUPD | GPIOCFG_PUPD_PDN);
    
    // configure and activate the SPI (master, internal slave select, software slave mgmt)
    // (use default mode: 8-bit, 2-wire, no crc, MSBF, PCLK/2, CPOL0, CPHA0)
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_SPE;
}

// perform SPI transaction with radio
u1_t hal_spi (u1_t out) {
    SPI1->DR = out;
    while( (SPI1->SR & SPI_SR_RXNE ) == 0);
    return SPI1->DR; // in
}

#ifdef CFG_lmic_clib

// -----------------------------------------------------------------------------
// TIME

static void hal_time_init () {
#ifndef CFG_clock_HSE
    PWR->CR |= PWR_CR_DBP; // disable write protect
    RCC->CSR |= RCC_CSR_LSEON; // switch on low-speed oscillator @32.768kHz
    while( (RCC->CSR & RCC_CSR_LSERDY) == 0 ); // wait for it...
#endif
    
    RCC->APB2ENR   |= RCC_APB2ENR_TIM9EN;     // enable clock to TIM9 peripheral 
    RCC->APB2LPENR |= RCC_APB2LPENR_TIM9LPEN; // enable clock to TIM9 peripheral also in low power mode
    RCC->APB2RSTR  |= RCC_APB2RSTR_TIM9RST;   // reset TIM9 interface
    RCC->APB2RSTR  &= ~RCC_APB2RSTR_TIM9RST;  // reset TIM9 interface

#if CFG_clock_HSE
    TIM9->PSC  = (640 - 1); // HSE_CLOCK_HWTIMER_PSC-1);  XXX: define HSE_CLOCK_HWTIMER_PSC somewhere
#else
    TIM9->SMCR = TIM_SMCR_ECE; // external clock enable (source clock mode 2) with no prescaler and no filter
#endif
    
    NVIC->IP[TIM9_IRQn] = 0x70; // interrupt priority
    NVIC->ISER[TIM9_IRQn>>5] = 1<<(TIM9_IRQn&0x1F);  // set enable IRQ

    // enable update (overflow) interrupt
    TIM9->DIER |= TIM_DIER_UIE;
    
    // Enable timer counting
    TIM9->CR1 = TIM_CR1_CEN;
}

u4_t hal_ticks () {
    hal_disableIRQs();
    u4_t t = HAL.ticks;
    u2_t cnt = TIM9->CNT;
    if( (TIM9->SR & TIM_SR_UIF) ) {
        // Overflow before we read CNT?
        // Include overflow in evaluation but
        // leave update of state to ISR once interrupts enabled again
        cnt = TIM9->CNT;
        t++;
    }
    hal_enableIRQs();
    return (t<<16)|cnt;
}

// return modified delta ticks from now to specified ticktime (0 for past, FFFF for far future)
static u2_t deltaticks (u4_t time) {
    u4_t t = hal_ticks();
    s4_t d = time - t;
    if( d<=0 ) return 0;    // in the past
    if( (d>>16)!=0 ) return 0xFFFF; // far ahead
    return (u2_t)d;
}

void hal_waitUntil (u4_t time) {
    while( deltaticks(time) != 0 ); // busy wait until timestamp is reached
}

// check and rewind for target time
u1_t hal_checkTimer (u4_t time) {
    u2_t dt;
    TIM9->SR &= ~TIM_SR_CC2IF; // clear any pending interrupts
    if((dt = deltaticks(time)) < 5) { // event is now (a few ticks ahead)
        TIM9->DIER &= ~TIM_DIER_CC2IE; // disable IE
        return 1;
    } else { // rewind timer (fully or to exact time))
        TIM9->CCR2 = TIM9->CNT + dt;   // set comparator
        TIM9->DIER |= TIM_DIER_CC2IE;  // enable IE
        TIM9->CCER |= TIM_CCER_CC2E;   // enable capture/compare uint 2
        return 0;
    }
}
  
void TIM9_IRQHandler () {
    if(TIM9->SR & TIM_SR_UIF) { // overflow
        HAL.ticks++;
    }
    if((TIM9->SR & TIM_SR_CC2IF) && (TIM9->DIER & TIM_DIER_CC2IE)) { // expired
        // do nothing, only wake up cpu
    }
    TIM9->SR = 0; // clear IRQ flags
}

// -----------------------------------------------------------------------------
// IRQ

void hal_disableIRQs () {
    __disable_irq();
    HAL.irqlevel++;
}

void hal_enableIRQs () {
    if(--HAL.irqlevel == 0) {
        __enable_irq();
    }
}

void hal_sleep () {
    // low power sleep mode
#ifndef CFG_no_low_power_sleep_mode
    PWR->CR |= PWR_CR_LPSDSR;
#endif
    // suspend execution until IRQ, regardless of the CPSR I-bit
    __WFI();
}

// -----------------------------------------------------------------------------

void hal_init () {
    memset(&HAL, 0x00, sizeof(HAL));
    hal_disableIRQs();

    // configure radio I/O and interrupt handler
    hal_io_init();
    // configure radio SPI
    hal_spi_init();
    // configure timer and interrupt handler
    hal_time_init();

    hal_enableIRQs();
}

void hal_failed () {
    // HALT...
    hal_disableIRQs();
    hal_sleep();
    while(1);
}

#endif // CFG_lmic_clib
