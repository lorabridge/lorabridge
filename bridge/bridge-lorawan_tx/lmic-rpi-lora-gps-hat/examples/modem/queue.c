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

#define NUMQ 5

static struct {
    u1_t* buf;
    u2_t len;
} queue[NUMQ];

static u1_t qhead;

void queue_init () {
    memset(queue, 0, sizeof(queue));
    qhead = 0;
}

void queue_add (u1_t* buf, u2_t len) {
    u1_t i, j;
    hal_disableIRQs();
    for(i=0, j=qhead; i<NUMQ; i++, j++) {
	if(j == NUMQ) {
	    j = 0;
	}
	if(queue[j].buf == NULL) {
	    queue[j].buf = buf;
	    queue[j].len = len;
	    break;
	}
    }
    if(i == NUMQ) {
	hal_failed();
    }
    hal_enableIRQs();
}

u1_t queue_shift (FRAME* f) {
    u1_t r = 0;
    hal_disableIRQs();
    if(queue[qhead].buf) {
	frame_init(f, queue[qhead].buf, queue[qhead].len);
	queue[qhead].buf = NULL;
	if(++qhead == NUMQ) {
	    qhead = 0;
	}
	r = 1;
    }
    hal_enableIRQs();
    return r;
}
