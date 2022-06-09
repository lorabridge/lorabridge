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

#define I2C_TIMEOUT_ticks 1000

#define I2C_BUS 1
#define I2C_SCL_PIN 8
#define I2C_SDA_PIN 9
#define I2Cx I2C1

// I2Cs are clocked by the APB1 clock, can run from 2MHz to 32MHz
// The I2C_APB1_FREQ sets up this frequency as a value in MHz
// (The I2C interface needs to know the speed of the bus in order to produce the 
// correct clock )
// Note: 
// Unfortunately, neither 8 MHz nor 32 MHz seemeed to work with the Semtech 
// boards ... just define this constant based on the real freq of the APB1 

#define I2C_APB1_FREQ 32

// APB1 frequency must be 
// - at least 2 MHz to achieve standard mode I²C frequencies
// - at least 4 MHz to achieve fast mode I²C frequencies
// - multiple of 10MHz to reach the 400 kHz maximum I²C fast mode clock
// I2C_DEFAULT_CCR is the default divider for the APB1_Freq
// In standard mode, I2C frequency can be computed as:
//   I2C_Freq = APB1_Freq / CCR
// Thus: 
//   400 kHz = 32 MHz / 80      -> CCR = 80
//   100 kHz = 32 MHz / 320     -> CCR = 320
// In fast mode, the computation is based on the duty cycle
// Assuming duty cycle is Tlow/Thigh = 2:1
// I2C_Freq = APB1_Freq / ( 2 * CCR + 1 * CCR )
// Thus:
//   400 kHz = 32 MHz / (3 * 26.666) -> rounding
//   CCR = 27 -> 395.062 kHz
//   CCR = 26 -> 410.256 kHz

// #define I2C_DEFAULT_CCR 320 // standard mode @ 100kHz
// #define I2C_DEFAULT_CCR 27 // fast mode @ 395kHz
#define I2C_DEFAULT_CCR (I2C_APB1_FREQ * 1000 / (3 * 400))

// -- Duty cycle / fast mode configuration
// FAST_MODE: Standard (0) or Fast mode (1) setup
// DUTY_CYCLE: Tlow/Thigh = 2:1 (0) or Tlow/Thigh = 16:9 (1)
// 16/9 mode is not implemented
#define I2C_FAST_MODE 1
#define I2C_DUTY_CYCLE 0

// TRISE - Nicely undocumented by STM32, one needs to refer
// to the I2C specification.
// For standard mode, just use I2C_APB1_FREQ + 1
// For fast mode, just do the same as STM does in their lib
// TRISE = I2C_APB1_FREQ * 300 / 1000 + 1;

#define I2C_DEFAULT_TRISE (((I2C_APB1_FREQ * 3) / 10) + 1 )
// #define I2C_DEFAULT_TRISE (I2C_APB1_FREQ + 1)  // standard speed

static u1_t i2c_isBusy () {
    // wait for bus to be free for max 20ms
    ostime_t timeout = os_getTime() + ms2osticks(20);
    while ((I2Cx->SR2 & I2C_SR2_BUSY) && os_getTime() < timeout); // wait...
    // if still busy, send STOP, something went terribly wrong!
    if (I2Cx->SR2 & I2C_SR2_BUSY) {
        I2Cx->CR1 |= I2C_CR1_STOP;
        return 1;
    }
    return 0; // bus free
}

// the addr has to be left aligned, with the read (aka not-write) bit set
static s1_t i2c_send_start (u1_t addr) {   
    I2Cx->CR1 |= I2C_CR1_START;        // send start condition
    
    u1_t timeout = 0xff;
    while(!(I2Cx->SR1 & I2C_SR1_SB) && timeout-- > 0); // wait for SB (start bit / start condition generated)     
    if (timeout == 0) return -1;
    
    I2Cx->DR = addr & I2C_DR_DR;      // send slave address
    
    timeout = 0xff;
    // address is being TXed
    while(!(I2Cx->SR1 & I2C_SR1_ADDR) && !(I2Cx->SR1 & I2C_SR1_AF) && timeout-- > 0);      
    if ( timeout == 0 || (I2Cx->SR1 & I2C_SR1_AF) ) {
        // the address is not on the bus
        I2Cx->SR1 &= ~I2C_SR1_AF; // make sure the flash is cleared
        return -1;
    }  
    return 0;
}

static s1_t i2c_restart_write (u1_t addr){
    if (i2c_send_start(addr) != 0) 
        return -1;
    // check that the start was ACKed
    if (!(I2Cx->SR2 & I2C_SR2_TRA)){
        // wait a bit
        hal_waitUntil(os_getTime()+us2osticks(50));
        if (!(I2Cx->SR2 & I2C_SR2_TRA))
            return -2; // utterly broken I2C (a STOP will be sent by i2c main loop)
    }
    // OK now
    return 0;
}

static s1_t i2c_restart_read (u1_t addr) {
    if (i2c_send_start(addr | 1) != 0)
        return -1;
    // check that the start was ACKed
    if (I2Cx->SR2 & I2C_SR2_TRA){
        // wait a bit
        hal_waitUntil(os_getTime()+us2osticks(50));
        if (I2Cx->SR2 & I2C_SR2_TRA)
            return -2; // utterly broken I2C (a STOP will be sent by i2c main loop)
    }
    // OK now
    return 0;
}

u1_t i2c_init (void) {
    // setup the PB10 and PB11 pins as OC outputs    
    // enable power to GPIOs
    RCC->AHBENR    |= RCC_AHBENR_GPIOBEN; 

#if I2C_BUS == 2 //TODO:xan: change to use "hw_cfg_pin" for the pin configuration!

    // select AF4 for the pins
    GPIOB->AFR[1]  |= 4 << 2*4 | 4 << 3*4 ; 
    
    // there are external pull-ups, no internal pulling
    GPIOB->PUPDR   &= ~( 3 << 10*2 | 3 << 11*2 );  
    
    // write 2 (= alternate function) in the MODER    
    GPIOB->MODER   |= 2 << 10*2 | 2 << 11*2;
    
    // set open drain on the pins    
    GPIOB->OTYPER  |= 1 << 10 | 1 << 11;
    GPIOB->OSPEEDR &= ~(3 << 10*2 | 3 << 11*2);  // 0 is default, no need to set                            

    // enable clock to the I2C
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;     

    // reseting the bus (optional)
    // I2Cx->CR1    |= I2C_CR1_SWRST; 
    // I2Cx->CR1    &= ~I2C_CR1_SWRST; 
    // while (I2Cx->CR1 & I2C_CR1_SWRST);    
    
    // disable I2C
    I2Cx->CR1 &= ~I2C_CR1_PE;
    
    // setup frequency
    I2Cx->CR2 |= I2C_APB1_FREQ & I2C_CR2_FREQ;      
    
    // the CCR must be configured when the I2C is disabled (PE=0)
    I2Cx->CCR |=  
        ((I2C_FAST_MODE << 15) & I2C_CCR_FS) |
        ((I2C_DUTY_CYCLE << 14) & I2C_CCR_DUTY) |
        ( I2C_DEFAULT_CCR & I2C_CCR_CCR);

    I2Cx->TRISE |= (I2C_DEFAULT_TRISE) & I2C_TRISE_TRISE; // max. rise time
    
    I2Cx->CR1 |= I2C_CR1_PE;                              // enable peripheral

#elif I2C_BUS == 1
    // set up the I2C1 pins
    
    if (I2Cx->SR2 & I2C_SR2_BUSY) {
        // brute force 'wobble clock'
        hw_cfg_pin(GPIOB,I2C_SCL_PIN,GPIOCFG_MODE_OUT|GPIOCFG_OSPEED_40MHz|GPIO_AF_I2C1|GPIOCFG_OTYPE_OPEN|GPIOCFG_PUPD_NONE);
        for (int i=0;i<64;i++) {        
            GPIOB->ODR |= (1<<I2C_SCL_PIN);
            GPIOB->ODR &= ~(1<<I2C_SCL_PIN);
        }   
    }
    
    // correct pin configuration
    hw_cfg_pin(GPIOB,I2C_SCL_PIN,GPIOCFG_MODE_ALT|GPIOCFG_OSPEED_40MHz|GPIO_AF_I2C1|GPIOCFG_OTYPE_OPEN|GPIOCFG_PUPD_NONE);
    hw_cfg_pin(GPIOB,I2C_SDA_PIN,GPIOCFG_MODE_ALT|GPIOCFG_OSPEED_40MHz|GPIO_AF_I2C1|GPIOCFG_OTYPE_OPEN|GPIOCFG_PUPD_NONE);    
    // enable the clock for the I2C1 module
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;        
    
    I2Cx->CR1 &= ~I2C_CR1_PE; // disable I2C peripheral
    
    // setup I2C frequency for SCL (must be done only when peripheral is disabled!)
    I2Cx->CR2  |= 32;  // we run at 32 MHz
    //I2Cx->CCR   = 160; // SCL 100 kHz
    I2Cx->CCR   = 40; // SCL 400 kHz
    I2Cx->TRISE = 33;  // max 1000 ns

    I2Cx->CR1 |= I2C_CR1_PE; // enable peripheral       
#endif    

    return 0;
}

s1_t i2c_xfer (u1_t addr, u1_t* data, u1_t wlen, u1_t rlen) {
    s1_t errc = 0;
    hal_disableIRQs();
    
    // check that the bus is not stuck a previous transaction
    if (i2c_isBusy()) { // will also try to 'mend' the bus
        errc = 0xDD;
        goto finish;
    }
    
    if (wlen > 0) {
        if ( (errc = i2c_restart_write(addr)) != 0)
            goto finish;
        
        for (u1_t i = 0; i < wlen; i++) {
            I2Cx->DR = data[i] & I2C_DR_DR;
            while(!(I2Cx->SR1 & I2C_SR1_BTF) && !(I2Cx->SR1 & I2C_SR1_TXE)); 
            if (I2Cx->SR1 & I2C_SR1_AF) { // no ACK
                I2Cx->SR1 &= ~I2C_SR1_AF; // clear flag
                errc = -1;
                goto finish;
            }
        }
        while( !(I2Cx->SR1 & I2C_SR1_BTF) ); // wait for the last byte to be transferred
    }
    
    hal_waitUntil(os_getTime() + us2osticks(50)); 
    
    if (rlen > 0){
        if ( (errc=i2c_restart_read(addr)) != 0)
            goto finish;
        
        for (u1_t i = 0; i < rlen; i++){
            // we always ACK the received data, except last byte
            if ( i == (rlen-1))
                I2Cx->CR1 &= ~I2C_CR1_ACK; 
            else
                I2Cx->CR1 |= I2C_CR1_ACK;
            
            u2_t counter = 0;
            while(!(I2Cx->SR1 & I2C_SR1_RXNE)) {
                if (counter++ > 50000) { // safety timeout, in case nothing is received
                    errc = -2;
                    goto finish;
                }
            }
            data[i] = I2Cx->DR;
        }
    }  
finish:
    I2Cx->CR1 |= I2C_CR1_STOP;
    hal_enableIRQs();
    return errc;
}
