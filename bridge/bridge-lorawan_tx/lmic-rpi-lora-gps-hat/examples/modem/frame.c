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
#include "modem.h"

FRAME rxframe;
FRAME txframe;

static osjob_t txjob;
static osjob_t rxjob;

void frame_init (FRAME* f, u1_t* buf, u2_t max) {
    f->state = 0;
    f->lrc = 0;
    f->len = 0;
    f->buf = buf;
    f->max = max;
}

// called by usart irq handler (return next char to send 0x00XX, or 0x0100)
u2_t frame_tx (u1_t next) {
    if(next) {
	return (txframe.len < txframe.max) ? txframe.buf[txframe.len++] : 0x100;
    } else { // complete
	os_setCallback(&txjob, modem_txdone); // run job
	return 0;
    }
}

// called by usart irq handler (pass received char, return 1 to continue rx, 0 to stop)
// ASCII format:  ATxxxxxxx\r
// Binary format: B%lxxxxxxxx%c
u1_t frame_rx (u1_t c) {
    switch(rxframe.state) {
    case FRAME_INIT:
	if(c == 'a' || c == 'A') {
	    rxframe.state = FRAME_A_A;
	} else if(c == 'B') {
	    rxframe.state = FRAME_B_B;
	    rxframe.lrc = c;
	} else {
	    // keep state, wait for sync
	}
	break;
    case FRAME_A_A:
	if(c == 't' || c == 'T') {
	    rxframe.state = FRAME_A_T;
	}
	break;
    case FRAME_A_T:
	if(c == '\r') {
	    rxframe.state = FRAME_A_OK;
	    os_setCallback(&rxjob, modem_rxdone); // run job
	    return 0; // stop reception
	} else {
	    if(rxframe.len == rxframe.max) { // overflow
		rxframe.state = FRAME_A_ERR;
		os_setCallback(&rxjob, modem_rxdone); // run job
		return 0; // stop reception
	    }
	    rxframe.buf[rxframe.len++] = c; // store cmd
	}
	break;
    case FRAME_B_B:
	if(c > rxframe.max) {
	    rxframe.state = FRAME_B_ERR;
	    os_setCallback(&rxjob, modem_rxdone); // run job
	    return 0; // stop reception
	}
	rxframe.max = c; // store len
	rxframe.lrc ^= c;
	rxframe.state = FRAME_B_LEN;
	break;
    case FRAME_B_LEN:
	rxframe.buf[rxframe.len++] = c; // store cmd
	rxframe.lrc ^= c;
	if(rxframe.len == rxframe.max) {
	    rxframe.state = FRAME_B_LRC;
	}
	break;
    case FRAME_B_LRC:
	if(rxframe.lrc ^ c) {
	    rxframe.state = FRAME_B_ERR;
	} else {
	    rxframe.state = FRAME_B_OK;
	}
	os_setCallback(&rxjob, modem_rxdone); // run job
	return 0; // stop reception
    }
    return 1; // continue to receive
}
