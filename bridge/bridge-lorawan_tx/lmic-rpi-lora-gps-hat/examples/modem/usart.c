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
#include "modem.h"

// USART1
#define USART             USART1
#define USART_TX_PORT     GPIOA
#define USART_TX_PIN      9
#define USART_RX_PORT     GPIOA
#define USART_RX_PIN      10
#define USART_AF          0x07
#define USART_RCCREG      APB2ENR
#define USART_RCCVAL      RCC_APB2ENR_USART1EN
#define USART_IRQN        USART1_IRQn
#define USART_IRQHANDLER  USART1_IRQHandler

void usart_init () {
    hal_disableIRQs();
    // configure USART (115200/8N1)
    RCC->USART_RCCREG |= USART_RCCVAL;
    hw_cfg_pin(USART_TX_PORT, USART_TX_PIN, GPIOCFG_MODE_ALT|GPIOCFG_OSPEED_40MHz|GPIOCFG_OTYPE_PUPD|GPIOCFG_PUPD_PUP|USART_AF);
    hw_cfg_pin(USART_RX_PORT, USART_RX_PIN, GPIOCFG_MODE_ALT|GPIOCFG_OSPEED_40MHz|GPIOCFG_OTYPE_PUPD|GPIOCFG_PUPD_PUP|USART_AF);
    USART->BRR = 277; // 115200
    // configure NVIC
    NVIC->IP[USART_IRQN]      = 0x70; // interrupt priority
    NVIC->ISER[USART_IRQN>>5] = 1 << (USART_IRQN&0x1F);  // set enable IRQ
    // enable usart
    USART->CR1 = USART_CR1_UE;
    hal_enableIRQs();
}

void usart_starttx () {
    USART->CR1 |= (USART_CR1_TE | USART_CR1_TXEIE);
}

void usart_startrx () {
    USART->CR1 |= (USART_CR1_RE | USART_CR1_RXNEIE);
}

void USART_IRQHANDLER (void) {
    hal_disableIRQs();

    // check status reg (clears most of the flags)
    u4_t sr = USART->SR;
    if (sr & (USART_SR_PE|USART_SR_ORE|USART_SR_FE))  {
	hal_failed();
    }

    // check for tx reg empty
    if( (USART->CR1 & USART_CR1_TXEIE) && (sr & USART_SR_TXE) ) {
	u2_t c = frame_tx(1);
	if((c & 0xFF00) == 0) { // send next char
	    USART->DR = c;
	} else { // no more chars - wait for completion
            USART->CR1 &= ~USART_CR1_TXEIE;
            USART->CR1 |= USART_CR1_TCIE;
        }
    }    

    // check for tx complete
    if( (USART->CR1 & USART_CR1_TCIE) && (sr & USART_SR_TC) ) {
        USART->CR1 &= ~(USART_CR1_TE | USART_CR1_TCIE); // stop transmitter
	frame_tx(0);
    }
    
    // check for rx reg not empty
    if( (USART->CR1 & USART_CR1_RXNEIE) && (sr & USART_SR_RXNE) ) {
	if(frame_rx(USART->DR) == 0) {
	    USART->CR1 &= ~(USART_CR1_RE | USART_CR1_RXNEIE); // stop receiver
	}
    }

    hal_enableIRQs();
}
