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

#define GPIO_AF_BITS        4     // width of bit field
#define GPIO_AF_MASK        0x0F  // mask in AFR[0/1]
#define GPIO_AFRLR(i)       ((i)>>3)
#define GPIO_AF_PINi(i,af)  ((af)<<(((i)&7)*GPIO_AF_BITS))
#define GPIO_AF_set(gpio,i,af) ((gpio)->AFR[GPIO_AFRLR(i)] = \
                                ((gpio)->AFR[GPIO_AFRLR(i)] \
                                 & ~GPIO_AF_PINi(i,GPIO_AF_MASK)) \
                                |   GPIO_AF_PINi(i,af))

void hw_cfg_pin (GPIO_TypeDef* gpioport, u1_t pin, u2_t gpiocfg) {
    u1_t pin2 = 2*pin;

    GPIO_AF_set(gpioport, pin, gpiocfg & GPIOCFG_AF_MASK);
    gpioport->MODER   = (gpioport->MODER   & ~(3 << pin2)) | (((gpiocfg >> GPIOCFG_MODE_SHIFT  ) & 3) << pin2);
    gpioport->OSPEEDR = (gpioport->OSPEEDR & ~(3 << pin2)) | (((gpiocfg >> GPIOCFG_OSPEED_SHIFT) & 3) << pin2);
    gpioport->OTYPER  = (gpioport->OTYPER  & ~(1 << pin )) | (((gpiocfg >> GPIOCFG_OTYPE_SHIFT ) & 1) << pin );
    gpioport->PUPDR   = (gpioport->PUPDR   & ~(3 << pin2)) | (((gpiocfg >> GPIOCFG_PUPD_SHIFT  ) & 3) << pin2);
}

void hw_set_pin (GPIO_TypeDef* gpioport, u1_t pin, u1_t state) {
    gpioport->ODR     = (gpioport->ODR     & ~(1 << pin))  | ((state & 1) << pin );
}

void hw_cfg_extirq (u1_t portidx, u1_t pin, u1_t irqcfg) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // make sure module is on

    // configure external interrupt (every irq line 0-15 can be configured with a 4-bit port index A-G)
    u4_t tmp1 = (pin & 0x3) << 2;
    u4_t tmp2 = ((u4_t)0x0F) << tmp1;
    SYSCFG->EXTICR[pin >> 2] = (SYSCFG->EXTICR[pin >> 2] & ~tmp2) | (((u4_t)portidx) << tmp1);

    // configure trigger and enable irq
    u4_t mask = (u4_t)(1 << pin);
    EXTI->RTSR &= ~mask; // clear trigger
    EXTI->FTSR &= ~mask; // clear trigger
    switch(irqcfg & GPIO_IRQ_MASK) {
      case GPIO_IRQ_RISING:   EXTI->RTSR |= mask; break; // trigger at rising edge
      case GPIO_IRQ_FALLING:  EXTI->FTSR |= mask; break; // trigger at falling edge
      case GPIO_IRQ_CHANGE:   EXTI->RTSR |= mask; EXTI->FTSR |= mask; break; // trigger at both edges
    }
    EXTI->IMR |= mask;   // enable IRQ (pin x for configured port)

    // configure the NVIC
    u1_t channel = (pin < 5) ? (EXTI0_IRQn+pin) : ((pin<10) ? EXTI9_5_IRQn : EXTI15_10_IRQn);
    NVIC->IP[channel] = 0x70; // interrupt priority
    NVIC->ISER[channel>>5] = 1<<(channel&0x1F);  // set enable IRQ
}
