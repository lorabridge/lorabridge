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

// ring buffer - append at end, free from beg

static u1_t buffer[1024];
static u1_t* beg;
static u1_t* end;
static u1_t* max;

void buffer_init () {
    beg = end = max = buffer;
}

u1_t* buffer_alloc (u2_t len) {
    u1_t* buf = NULL;
    hal_disableIRQs();
    if(beg <= end) { // .......******...
	if(end + len < buffer + sizeof(buffer)) { // append
	    buf = end;
	    end += len;
	    max = end;
	} else { // wrap
	    if(buffer + len < beg) {
		buf = buffer;
		end = buffer + len;
	    }
	}
    } else { // ***.......*****.
	if(end + len < beg) {
	    buf = end;
	    end += len;
	}
    }
    hal_enableIRQs();
    return buf;
}

void buffer_free (u1_t* buf, u2_t len) {
    if(buf >= buffer && buf+len < buffer+sizeof(buffer)) {
	hal_disableIRQs();
	while(buf != beg); // halt if trying to free not from beginning
	if(beg <= end) { // .......******...
	    beg += len;
	    if(beg == end) { // empty
		beg = end = max = buffer; // reset
	    }
	} else { // ***.......*****.
	    beg += len;
	    if(beg == max) {
		beg = buffer;
		max = end;
	    }
	}
	hal_enableIRQs();
    }
}
